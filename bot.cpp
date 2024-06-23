#include <bits/stdc++.h>

static const int NUM_GAMES = 250;
static const int BET_SIZE = 10;
static const int MAX_BET = 500;
static const int CASH_FACTOR = 10;
static const int MCTS_ITERATIONS = 5000;
static const int NUM_PLAYERS = 5;

enum Suit { HEARTS, DIAMONDS, CLUBS, SPADES };
enum Rank { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };
enum Phase { EXCHANGE, BIDDING, SHOWDOWN };

struct Card {
    Suit suit;
    Rank rank;
    Card(Suit s, Rank r) : suit(s), rank(r) {}

    bool operator<(const Card& other) const {
        if (rank != other.rank) {
            return rank < other.rank;
        }
        return suit < other.suit;
    }
};

enum ActionType {
    DRAW,
    CALL,
    RAISE,
    FOLD
};

enum HandValue { HIGH_CARD, PAIR, TWO_PAIR, THREE_KIND, STRAIGHT, FLUSH, FULL_HOUSE, FOUR_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH };

struct BotParameters { // Dla botów na benchu
    double drawProbability;
    double callProbability;
    double raiseProbability;
    double foldProbability;
};

struct AdvancedBotParameters { // Na benchu 
    double drawProbabilityLow;
    double drawProbabilityHigh;
    double callProbability;
    double raiseProbability;
    double foldProbability;
};

struct Move {
    ActionType action;
    int amount; // Kwota zakładu
    std::set<int> cardsToDiscard;

    Move(ActionType act = DRAW, int amt = 10, std::set<int> discard = {}) : action(act), amount(amt), cardsToDiscard(discard) {}
};

struct HandEvaluation {
    HandValue value;
    std::vector<Rank> ranks; 

    HandEvaluation(HandValue v, std::vector<Rank> r) : value(v), ranks(r) {}

    bool operator<(const HandEvaluation& other) const {
        if (value != other.value)
            return value < other.value;

        for (size_t i = 0; i < ranks.size() && i < other.ranks.size(); ++i) {
            if (ranks[i] != other.ranks[i])
                return ranks[i] < other.ranks[i];
        }
        return false;
    }

    bool operator>(const HandEvaluation& other) const {
        return other < *this;
    }

    bool operator==(const HandEvaluation& other) const {
        return value == other.value && ranks == other.ranks;
    }
};

struct Hand {
    std::vector<Card> cards;
    void addCard(const Card& card) {
        cards.push_back(card);
    }
    HandEvaluation evaluateHandStrength() const {
        std::vector<Suit> suits;
        std::vector<Rank> ranks;
        std::map<Rank, int> rankCount;
        std::map<Suit, int> suitCount;

        for (const auto& card : cards) {
            suits.push_back(card.suit);
            ranks.push_back(card.rank);
            rankCount[card.rank]++;
            suitCount[card.suit]++;
        }

        std::sort(ranks.begin(), ranks.end(), std::greater<Rank>());

        bool isFlush = false;
        for (const auto& suit : suitCount) {
            if (suit.second == 5) {
                isFlush = true;
                break;
            }
        }

        bool isStraight = false;
        if (rankCount.size() == 5 && (ranks[0] - ranks[4] == 4 || (ranks[0] == ACE && ranks[1] == FIVE))) {
            isStraight = true;
        }

        if (isFlush && isStraight) {
            if (ranks[0] == ACE && ranks[1] == KING) {
                return HandEvaluation(ROYAL_FLUSH, ranks);
            }
            return HandEvaluation(STRAIGHT_FLUSH, ranks);
        }

        for (const auto& rank : rankCount) {
            if (rank.second == 4) {
                std::vector<Rank> sortedRanks = {rank.first};
                for (const auto& r : ranks) {
                    if (r != rank.first) {
                        sortedRanks.push_back(r);
                    }
                }
                return HandEvaluation(FOUR_KIND, sortedRanks);
            }
        }

        bool hasThree = false;
        bool hasPair = false;
        Rank threeRank, pairRank;
        for (const auto& rank : rankCount) {
            if (rank.second == 3) {
                hasThree = true;
                threeRank = rank.first;
            }
            if (rank.second == 2) {
                hasPair = true;
                pairRank = rank.first;
            }
        }

        if (hasThree && hasPair) {
            return HandEvaluation(FULL_HOUSE, {threeRank, pairRank});
        }

        if (isFlush) {
            return HandEvaluation(FLUSH, ranks);
        }

        if (isStraight) {
            return HandEvaluation(STRAIGHT, ranks);
        }

        if (hasThree) {
            std::vector<Rank> sortedRanks = {threeRank};
            for (const auto& r : ranks) {
                if (r != threeRank) {
                    sortedRanks.push_back(r);
                }
            }
            return HandEvaluation(THREE_KIND, sortedRanks);
        }

        std::vector<Rank> pairs;
        for (const auto& rank : rankCount) {
            if (rank.second == 2) {
                pairs.push_back(rank.first);
            }
        }

        if (pairs.size() == 2) {
            std::sort(pairs.begin(), pairs.end(), std::greater<Rank>());
            std::vector<Rank> sortedRanks = pairs;
            for (const auto& r : ranks) {
                if (std::find(pairs.begin(), pairs.end(), r) == pairs.end()) {
                    sortedRanks.push_back(r);
                }
            }
            return HandEvaluation(TWO_PAIR, sortedRanks);
        }

        if (pairs.size() == 1) {
            std::vector<Rank> sortedRanks = {pairs[0]};
            for (const auto& r : ranks) {
                if (r != pairs[0]) {
                    sortedRanks.push_back(r);
                }
            }
            return HandEvaluation(PAIR, sortedRanks);
        }

        return HandEvaluation(HIGH_CARD, ranks);
    }

};

class Deck {
    std::vector<Card> cards;
public:
    Deck() {
        for (int s = HEARTS; s <= SPADES; ++s) {
            for (int r = TWO; r <= ACE; ++r) {
                cards.emplace_back(static_cast<Suit>(s), static_cast<Rank>(r));
            }
        }
        std::shuffle(cards.begin(), cards.end(), std::mt19937{ std::random_device{}() });
    }
    Card drawCard() {
        Card card = cards.back();
        cards.pop_back();
        return card;
    }

    
    void remove(const std::set<Card>& excludedCards) {
        cards.erase(std::remove_if(cards.begin(), cards.end(), [&excludedCards](const Card& card) {
            return excludedCards.find(card) != excludedCards.end();
        }), cards.end());
    }

    void shuffle() {
        std::shuffle(cards.begin(), cards.end(), std::mt19937{ std::random_device{}() });
    }

    const std::vector<Card>& getCards() const {
        return cards;
    }
};

struct GameState {
    std::vector<Hand> playersHands;
    std::vector<int> bets;
    std::vector<bool> activePlayers;
    std::vector<int> wallets;
    int currentPlayer;
    int lastRaiser;
    int pot;
    int currentBet;
    Phase phase;
    Deck deck;

    GameState(int NUM_PLAYERS) : currentPlayer(0), lastRaiser(-1), pot(0), currentBet(BET_SIZE), phase(EXCHANGE) {
        playersHands.resize(NUM_PLAYERS);
        bets.resize(NUM_PLAYERS, BET_SIZE);
        activePlayers.resize(NUM_PLAYERS, true);
        wallets.resize(NUM_PLAYERS, NUM_GAMES * BET_SIZE);
    }

    void print() const {
        static const std::map<Rank, std::string> rankToString = {
            {TWO, "TWO"}, {THREE, "THREE"}, {FOUR, "FOUR"}, {FIVE, "FIVE"}, {SIX, "SIX"},
            {SEVEN, "SEVEN"}, {EIGHT, "EIGHT"}, {NINE, "NINE"}, {TEN, "TEN"}, {JACK, "JACK"},
            {QUEEN, "QUEEN"}, {KING, "KING"}, {ACE, "ACE"}
        };
        
        static const std::map<Suit, std::string> suitToString = {
            {HEARTS, "HEARTS"}, {DIAMONDS, "DIAMONDS"}, {CLUBS, "CLUBS"}, {SPADES, "SPADES"}
        };

        std::cout << "Current Player: " << currentPlayer << std::endl;
        std::cout << "Current Bet: " << currentBet << std::endl;
        std::cout << "Pot: " << pot << std::endl;
        std::cout << "Phase: " << (phase == EXCHANGE ? "Exchange" : (phase == BIDDING ? "Bidding" : "Showdown")) << std::endl;
        
        for (size_t i = 0; i < playersHands.size(); ++i) {
            std::cout << "Player " << i << ": ";
            std::cout << "Active: " << (activePlayers[i] ? "Yes" : "No") << ", ";
            std::cout << "Wallet: " << wallets[i] << ", ";
            std::cout << "Bet: " << bets[i] << ", ";
            std::cout << "Hand: ";
            for (const auto& card : playersHands[i].cards) {
                std::cout << "(" << rankToString.at(card.rank) << " of " << suitToString.at(card.suit) << ") ";
            }
            std::cout << std::endl;
        }
    }

    bool isTerminal() const {
        return phase == SHOWDOWN;
    }

    int getCurrentPlayer() const {
        return currentPlayer;
    }

    void applyMove(const Move& move) {
        if (move.action == DRAW) {
            for (int index : move.cardsToDiscard) {
                playersHands[currentPlayer].cards[index] = deck.drawCard();
            }

            wallets[currentPlayer] -= BET_SIZE;
            pot += BET_SIZE;

            if (currentPlayer == static_cast<int>(playersHands.size()) - 1) {
                phase = BIDDING;
                return;
            }
        } else if (move.action == CALL || (move.action == RAISE && currentBet == MAX_BET)) {
            int callAmount = currentBet - bets[currentPlayer];
            bets[currentPlayer] += callAmount;
            wallets[currentPlayer] -= callAmount;
            pot += callAmount;
            if (lastRaiser == -1) {
                lastRaiser = currentPlayer;
            }
        } else if (move.action == RAISE) {
            int raiseAmount = move.amount;
            int totalBet = currentBet + raiseAmount;
            int callAmount = totalBet - bets[currentPlayer];
            bets[currentPlayer] += callAmount;
            wallets[currentPlayer] -= callAmount;
            pot += callAmount;
            currentBet = totalBet;
            lastRaiser = currentPlayer;
        } else if (move.action == FOLD) {
            activePlayers[currentPlayer] = false;
        }
        
        // Przejście do następnego gracza
        bool looped = false;
        currentPlayer = (currentPlayer + 1) % playersHands.size();
        while (!activePlayers[currentPlayer]) {
            currentPlayer = (currentPlayer + 1) % playersHands.size();
        }

        if (currentPlayer == lastRaiser) {
            looped = true;
        }

        if (phase == BIDDING) {
            int activeCount = 0;
            for (size_t i = 0; i < activePlayers.size(); ++i) {
                if (activePlayers[i]) {
                    activeCount++;
                }
            }
            if (looped || activeCount <= 1) {
                phase = SHOWDOWN;
            }
        }
    }

    std::vector<Move> getPossibleMoves() const {
        std::vector<Move> possibleMoves;

        if (phase == EXCHANGE) {
            for (int i = 0; i < (1 << 5); ++i) {
                std::set<int> cardsToDiscard;
                for (int j = 0; j < 5; ++j) {
                    if (i & (1 << j)) {
                        cardsToDiscard.insert(j);
                    }
                }
                possibleMoves.push_back(Move(DRAW, 0, cardsToDiscard));
            }
        } else if (phase == BIDDING) {
            possibleMoves.push_back(Move(CALL));
            possibleMoves.push_back(Move(RAISE, BET_SIZE));
            possibleMoves.push_back(Move(FOLD));
        }

        return possibleMoves;
    }

    std::vector<int> getResult() const {
        HandEvaluation bestHand(HIGH_CARD, {});
        std::vector<int> winners;

        for (size_t i = 0; i < playersHands.size(); ++i) {
            if (activePlayers[i]) {
                HandEvaluation currentHand = playersHands[i].evaluateHandStrength();
                if (winners.empty() || currentHand > bestHand) {
                    bestHand = currentHand;
                    winners.clear();
                    winners.push_back(static_cast<int>(i));
                } else if (currentHand == bestHand) {
                    winners.push_back(static_cast<int>(i));
                }
            }
        }
        return winners;
    }

    void updateWallets() {
        auto winners = getResult();
        int splitPot = pot / winners.size();
        for (int winner : winners) {
            wallets[winner] += splitPot;
        }
    }
};

struct Node {
    GameState state;
    Node* parent;
    std::vector<Node*> children;
    Move move;
    int wins, visits;

    Node(GameState s, Node* p = nullptr, Move m = Move()) : state(s), parent(p), move(m), wins(0), visits(0) {}

    ~Node() {
        for (auto child: children) {
            delete child;
        }
    }
};

struct HeuristicParameters {
    double explorationConstant;
    double drawBiasLow;
    double drawBiasHigh;
    double callBiasBase;
    double raiseBiasBase;
    double handStrengthWeight;
    double bankrollWeight;
    std::map<HandValue, double> handStrengthWeights;

    HeuristicParameters(double ec, double dbl, double dbh, double cb, double rb, double hsw, double bw, std::map<HandValue, double> hswMap)
        : explorationConstant(ec), drawBiasLow(dbl), drawBiasHigh(dbh), callBiasBase(cb), raiseBiasBase(rb), 
          handStrengthWeight(hsw), bankrollWeight(bw), handStrengthWeights(hswMap) {}
};

double UCTValue(int totalVisit, double nodeWinScore, int nodeVisit, const HeuristicParameters& params) {
    if (nodeVisit == 0) {
        return std::numeric_limits<double>::max();
    }
    return (nodeWinScore / (double)nodeVisit) +
           params.explorationConstant * sqrt(log(totalVisit) / (double)nodeVisit);
}

class MCTS {
    HeuristicParameters params;
    Node* root;
    std::mt19937 generator;
    int playerNumber;

public:
    MCTS(GameState rootState, HeuristicParameters hp, int playerNum) : params(hp), root(new Node(rootState, nullptr, Move())), generator(std::random_device{}()), playerNumber(playerNum) {}

    void setGameState(const GameState& newState) {
        delete root;
        root = new Node(newState, nullptr, Move());
    }

    void runSearch(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            Node* node = select(root);
            std::vector<int> result = simulate(node->state, params);
            backpropagate(node, result);
        }
    }

    Node* select(Node* node) {
        while (!node->children.empty()) {
            node = *std::max_element(node->children.begin(), node->children.end(), [this](Node* a, Node* b) {
                return UCTValue(a->parent->visits, a->wins, a->visits, this->params) <
                       UCTValue(b->parent->visits, b->wins, b->visits, this->params);
            });
        }
        if (node->visits > 0) {
            expand(node);
        }
        return node;
    }

    void expand(Node* node) {
        for (const auto& move : node->state.getPossibleMoves()) {
            GameState newState = node->state;
            newState.applyMove(move);
            node->children.push_back(new Node(newState, node, move));
        }
    }

    std::vector<int> simulate(GameState state, const HeuristicParameters& params) {
        std::set<Card> excludedCards;
        for (const auto& card : state.playersHands[playerNumber].cards) {
            excludedCards.insert(card);
        }

        Deck simulationDeck;
        simulationDeck.remove(excludedCards);
        simulationDeck.shuffle();

        for (auto& hand : state.playersHands) {
            if (hand.cards.empty()) {
                for (int i = 0; i < 5; ++i) {
                    hand.addCard(simulationDeck.drawCard());
                }
            }
        }

        AdvancedBotParameters botParams = {0.3, 0.6, 0.2, 0.1, 0.1}; // Ustawienia dla przeciwników
        std::random_device rd;
        std::mt19937 gen(rd());

        while (!state.isTerminal()) {
            int currentPlayer = state.getCurrentPlayer();
            auto moves = state.getPossibleMoves();
            std::vector<double> moveProbabilities(moves.size(), 1.0);

            if (currentPlayer == playerNumber) {
                // Główny bot
                for (size_t i = 0; i < moves.size(); ++i) {
                    double probability = 1.0;
                    HandEvaluation handEvaluation = state.playersHands[currentPlayer].evaluateHandStrength();
                    HandValue handValue = handEvaluation.value;

                    if (moves[i].action == DRAW) {
                        if (handValue <= TWO_PAIR) {
                            probability *= params.drawBiasLow;
                        } else {
                            probability *= params.drawBiasHigh;
                        }
                    } else if (moves[i].action == CALL) {
                        probability *= params.callBiasBase * (1.0 + params.handStrengthWeights.at(handValue));
                    } else if (moves[i].action == RAISE) {
                        probability *= params.raiseBiasBase * (1.0 + params.handStrengthWeights.at(handValue));
                    }

                    if (state.phase != EXCHANGE) {
                        probability *= (1.0 + params.handStrengthWeight * handEvaluation.value);
                    }

                    double bankrollFactor = 1.0 + params.bankrollWeight * state.bets[currentPlayer];
                    probability *= bankrollFactor;

                    // Uwzględnienie kosztów gry
                    probability *= 1.0 + (double(state.wallets[currentPlayer]) / (state.wallets[currentPlayer] + 10));

                    moveProbabilities[i] = probability;
                }
            } else {
                // Przeciwnicy
                HandEvaluation handEvaluation = state.playersHands[currentPlayer].evaluateHandStrength();
                double handStrength = static_cast<double>(handEvaluation.value);

                for (size_t i = 0; i < moves.size(); ++i) {
                    switch (moves[i].action) {
                        case DRAW:
                            moveProbabilities[i] *= (handStrength <= TWO_PAIR) ? botParams.drawProbabilityLow : botParams.drawProbabilityHigh;
                            break;
                        case CALL:
                            moveProbabilities[i] *= botParams.callProbability;
                            break;
                        case RAISE:
                            moveProbabilities[i] *= botParams.raiseProbability;
                            break;
                        case FOLD:
                            moveProbabilities[i] *= botParams.foldProbability;
                            break;
                    }
                }
            }

            std::discrete_distribution<int> dist(moveProbabilities.begin(), moveProbabilities.end());
            int chosenMoveIndex = dist(gen);
            state.applyMove(moves[chosenMoveIndex]);
        }

        return state.getResult();
    }

    void backpropagate(Node* node, const std::vector<int>& result) {
        while (node != nullptr) {
            node->visits++;
            if (std::find(result.begin(), result.end(), playerNumber) != result.end()) {
                node->wins++;
            }
            node = node->parent;
        }
    }

    Move getBestMove() const {
        return (*std::max_element(root->children.begin(), root->children.end(), [](Node* a, Node* b) {
            return a->visits < b->visits;
        }))->move;
    }
};

void randomizeBotPositions(std::vector<int>& positions) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(positions.begin(), positions.end(), g);
}

void playGameRandom(int numGames, bool verbose) {
    numGames *= 100;

    std::vector<int> wallets(NUM_PLAYERS, numGames * MAX_BET / CASH_FACTOR);
    std::random_device rd;
    std::mt19937 gen(rd());
    int max = 0, max_cash = -1;
    std::vector<int> botPositions = {0, 1, 2, 3, 4};

    for (int game = 0; game < numGames; ++game) {
        randomizeBotPositions(botPositions);

        std::vector<int> permutedWallets(NUM_PLAYERS);
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            permutedWallets[i] = wallets[botPositions[i]];
        }

        GameState initialState(NUM_PLAYERS);
        initialState.wallets = permutedWallets;

        Deck deck;
        for (auto& hand : initialState.playersHands) {
            for (int i = 0; i < 5; ++i) {
                hand.addCard(deck.drawCard());
            }
        }
        initialState.deck = deck;

        while (!initialState.isTerminal()) {
            auto possibleMoves = initialState.getPossibleMoves();
            std::uniform_int_distribution<int> dist(0, possibleMoves.size() - 1);
            int chosenMoveIndex = dist(gen);
            initialState.applyMove(possibleMoves[chosenMoveIndex]);
        }

        initialState.updateWallets();
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            wallets[botPositions[i]] = initialState.wallets[i];
        }

        if (verbose) initialState.print();

        auto winners = initialState.getResult();
        std::cout << std::endl << "󰠰 Game " << game + 1 << " 󰡶 Winners: ";
        for (int winner : winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < NUM_PLAYERS; ++i) {
            std::cout << "Player 󱚟 " << i + 1 << " (Position " << botPositions[i] + 1 << ") wallet: " << wallets[i] << " 󰄔 " << std::endl;
        }
    }

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (wallets[i] > max_cash) {
            max = i;
            max_cash = wallets[i];
        }
    }

    std::cout << std::endl << " Overall winner: Player 󱚟 " << max + 1 << std::endl;
}

Move getRandomMove(const GameState& state, const BotParameters& params, std::mt19937& gen) { // Dla losowych botów
    auto possibleMoves = state.getPossibleMoves();
    std::vector<double> moveProbabilities(possibleMoves.size(), 1.0);
    
    for (size_t i = 0; i < possibleMoves.size(); ++i) {
        switch (possibleMoves[i].action) {
            case DRAW:
                moveProbabilities[i] *= params.drawProbability;
                break;
            case CALL:
                moveProbabilities[i] *= params.callProbability;
                break;
            case RAISE:
                moveProbabilities[i] *= params.raiseProbability;
                break;
            case FOLD:
                moveProbabilities[i] *= params.foldProbability;
                break;
        }
    }

    std::discrete_distribution<int> dist(moveProbabilities.begin(), moveProbabilities.end());
    int chosenMoveIndex = dist(gen);
    return possibleMoves[chosenMoveIndex];
}

struct BasicBotParameters {
    double drawThreshold;
    double callThreshold;
    double raiseThreshold;
};

Move getBasicBotMove(const GameState& state, const BasicBotParameters& params, std::mt19937& gen) {
    auto possibleMoves = state.getPossibleMoves();
    HandEvaluation handEval = state.playersHands[state.getCurrentPlayer()].evaluateHandStrength();
    double handStrength = static_cast<double>(handEval.value);

    for (const auto& move : possibleMoves) {
        if (move.action == DRAW && handStrength < params.drawThreshold) {
            return move;
        } else if (move.action == CALL && handStrength < params.callThreshold) {
            return move;
        } else if (move.action == RAISE && handStrength >= params.raiseThreshold) {
            return move;
        } else if (move.action == FOLD && handStrength < params.callThreshold) {
            return move;
        }
    }

    // Fallback to a random move if no specific condition is met
    std::uniform_int_distribution<int> dist(0, possibleMoves.size() - 1);
    return possibleMoves[dist(gen)];
}

Move getAdvancedBotMove(const GameState& state, const AdvancedBotParameters& params, std::mt19937& gen) {
    auto possibleMoves = state.getPossibleMoves();
    HandEvaluation handEval = state.playersHands[state.getCurrentPlayer()].evaluateHandStrength();
    double handStrength = static_cast<double>(handEval.value);

    std::vector<double> moveProbabilities(possibleMoves.size(), 1.0);

    for (size_t i = 0; i < possibleMoves.size(); ++i) {
        switch (possibleMoves[i].action) {
            case DRAW:
                moveProbabilities[i] *= (handStrength <= TWO_PAIR) ? params.drawProbabilityLow : params.drawProbabilityHigh;
                break;
            case CALL:
                moveProbabilities[i] *= params.callProbability;
                break;
            case RAISE:
                moveProbabilities[i] *= params.raiseProbability;
                break;
            case FOLD:
                moveProbabilities[i] *= params.foldProbability;
                break;
        }
    }

    std::discrete_distribution<int> dist(moveProbabilities.begin(), moveProbabilities.end());
    int chosenMoveIndex = dist(gen);
    return possibleMoves[chosenMoveIndex];
}

void playGameBenchmark(int numGames, bool verbose, const std::vector<HeuristicParameters>& agentsParams) {
    std::vector<double> fide;

    std::vector<AdvancedBotParameters> botParams = {
        {0.4, 0.6, 0.2, 0.1, 0.1}, 
        {0.3, 0.5, 0.3, 0.1, 0.1}, 
        {0.2, 0.4, 0.4, 0.1, 0.1}, 
        {0.1, 0.3, 0.4, 0.2, 0.2}  
    };

    for (int id = 0; id < static_cast<int>(agentsParams.size()); ++id) {
        std::vector<int> wallets(NUM_PLAYERS, numGames * MAX_BET / CASH_FACTOR);
        std::vector<int> botPositions = {0, 1, 2, 3, 4};
        std::random_device rd;
        std::mt19937 gen(rd());

        for (int game = 0; game < numGames; ++game) {
            randomizeBotPositions(botPositions);
            std::vector<int> permutedWallets(NUM_PLAYERS);
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                permutedWallets[i] = wallets[botPositions[i]];
            }
            GameState initialState(NUM_PLAYERS);
            initialState.wallets = permutedWallets;

            Deck deck;
            for (auto& hand : initialState.playersHands) {
                for (int i = 0; i < 5; ++i) {
                    hand.addCard(deck.drawCard());
                }
            }
            initialState.deck = deck;

            MCTS mctsBot(initialState, agentsParams[id], botPositions[0]);

            while (!initialState.isTerminal()) {
                int currentPlayer = initialState.getCurrentPlayer();
                if (botPositions[0] == currentPlayer) {
                    mctsBot.setGameState(initialState);
                    mctsBot.runSearch(MCTS_ITERATIONS);
                    Move bestMove = mctsBot.getBestMove();
                    initialState.applyMove(bestMove);
                } else {
                    int botIndex = currentPlayer - 1;
                    Move advancedBotMove = getAdvancedBotMove(initialState, botParams[botIndex], gen);
                    initialState.applyMove(advancedBotMove);
                }
            }

            initialState.updateWallets();
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                wallets[botPositions[i]] = initialState.wallets[i];
            }

            if (verbose) initialState.print();

            auto winners = initialState.getResult();
            std::cout << std::endl << "󰠰 Game " << game + 1 << " 󰡶 Winners: ";
            for (int winner : winners) {
                std::cout << winner << " ";
            }
            std::cout << std::endl;
            std::cout << "Agent 󱚝 A" << id + 1 << " (Position " << botPositions[0] + 1 << ") wallet: " << wallets[0] << " 󰄔 " << std::endl;

            for (int i = 1; i < NUM_PLAYERS; ++i) {
                std::cout << "Bot  " << i << " (Position " << botPositions[i] + 1 << ") wallet: " << wallets[i] << " 󰄔 " << std::endl;
            }
        }
        fide.push_back((double)(wallets[0] - numGames * MAX_BET / CASH_FACTOR) / (double)(numGames * MAX_BET / CASH_FACTOR));
    }
    std::cout << std::endl << "Fide results: " << std::endl;

    for (int i = 0; i < static_cast<int>(agentsParams.size()); i++) {
        std::cout << "Agent 󱚝 A" << i + 1 << " Fide: " << fide[i] << "󱦹" << std::endl;
    }
}

std::vector<HeuristicParameters> generateParameterSets() {
    std::vector<HeuristicParameters> parameterSets;
    std::map<HandValue, double> handStrengthWeights = {
        {HIGH_CARD, 0.1}, {PAIR, 0.2}, {TWO_PAIR, 0.4}, {THREE_KIND, 0.6},
        {STRAIGHT, 0.8}, {FLUSH, 1.0}, {FULL_HOUSE, 1.2}, {FOUR_KIND, 1.4},
        {STRAIGHT_FLUSH, 1.6}, {ROYAL_FLUSH, 2.0}
    };

    for (double ec : {0.5, 1.0, 1.5}) {
        for (double dbl : {0.1, 0.2, 0.3}) {
            for (double dbh : {0.1, 0.2, 0.3}) {
                for (double cb : {0.5, 1.0, 1.5}) {
                    for (double rb : {0.5, 1.0, 1.5}) {
                        for (double hsw : {0.5, 1.0, 1.5}) {
                            for (double bw : {0.5, 1.0, 1.5}) {
                                parameterSets.emplace_back(
                                    ec, dbl, dbh, cb, rb, hsw, bw, handStrengthWeights
                                );
                            }
                        }
                    }
                }
            }
        }
    }
    return parameterSets;
}

void evaluateParameterSets(int numGames) {
    auto parameterSets = generateParameterSets();

    playGameBenchmark(numGames, false, parameterSets);
}

void playGameShowdown(int numGames, bool verbose, const std::vector<HeuristicParameters>& agentsParams) {
    std::vector<int> wallets(NUM_PLAYERS, numGames * MAX_BET / CASH_FACTOR);
    int max = 0, max_cash = -1;

    std::vector<int> botPositions = {0, 1, 2, 3, 4};

    for (int game = 0; game < numGames; ++game) {
        randomizeBotPositions(botPositions);
        std::vector<int> permutedWallets(NUM_PLAYERS);
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            permutedWallets[i] = wallets[botPositions[i]];
        }
        GameState initialState(NUM_PLAYERS);
        initialState.wallets = permutedWallets;

        Deck deck;
        for (auto& hand : initialState.playersHands) {
            for (int i = 0; i < 5; ++i) {
                hand.addCard(deck.drawCard());
            }
        }
        initialState.deck = deck;

        std::vector<MCTS> mctsBots;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            mctsBots.emplace_back(initialState, agentsParams[i], botPositions[i]);
        }

        while (!initialState.isTerminal()) {
            int currentPlayer = initialState.getCurrentPlayer();
            mctsBots[botPositions[currentPlayer]].setGameState(initialState);
            mctsBots[botPositions[currentPlayer]].runSearch(MCTS_ITERATIONS);
            Move bestMove = mctsBots[botPositions[currentPlayer]].getBestMove();
            initialState.applyMove(bestMove);
        }

        initialState.updateWallets();
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            wallets[botPositions[i]] = initialState.wallets[i];
        }

        if (verbose) initialState.print();

        auto winners = initialState.getResult();
        std::cout << std::endl << "󰠰 Game " << game + 1 << " 󰡶 Winners: ";
        for (int winner : winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < NUM_PLAYERS; ++i) {
            std::cout << "Agent 󱚝 A" << botPositions[i] + 1 << " (Position " << botPositions[i] + 1 << ") wallet: " << wallets[botPositions[i]] << " 󰄔 " << std::endl;
        }
    }

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (wallets[i] > max_cash) {
            max = i;
            max_cash = wallets[i];
        }
    }

    std::cout << std::endl << " Overall winner: Agent 󱚝 A" << max + 1 << std::endl;
}

HeuristicParameters readParams(std::ifstream& file) {
    double ec, dbl, dbh, cb, rb, hsw, bw;
    std::map<HandValue, double> hswMap;
    std::string line, key;

    std::getline(file, line); // Ignorowanie nazwy agenta

    while (std::getline(file, line) && !line.empty()) {
        std::istringstream iss(line);
        iss >> key;
        if (key == "explorationConstant") {
            iss >> ec;
        } else if (key == "drawBiasLow") {
            iss >> dbl;
        } else if (key == "drawBiasHigh") {
            iss >> dbh;
        } else if (key == "callBiasBase") {
            iss >> cb;
        } else if (key == "raiseBiasBase") {
            iss >> rb;
        } else if (key == "handStrengthWeight") {
            iss >> hsw;
        } else if (key == "bankrollWeight") {
            iss >> bw;
        } else if (key == "handStrengthWeights") {
            std::string hand;
            double weight;
            while (iss >> hand >> weight) {
                if (hand == "HIGH_CARD") hswMap[HIGH_CARD] = weight;
                else if (hand == "PAIR") hswMap[PAIR] = weight;
                else if (hand == "TWO_PAIR") hswMap[TWO_PAIR] = weight;
                else if (hand == "THREE_KIND") hswMap[THREE_KIND] = weight;
                else if (hand == "STRAIGHT") hswMap[STRAIGHT] = weight;
                else if (hand == "FLUSH") hswMap[FLUSH] = weight;
                else if (hand == "FULL_HOUSE") hswMap[FULL_HOUSE] = weight;
                else if (hand == "FOUR_KIND") hswMap[FOUR_KIND] = weight;
                else if (hand == "STRAIGHT_FLUSH") hswMap[STRAIGHT_FLUSH] = weight;
                else if (hand == "ROYAL_FLUSH") hswMap[ROYAL_FLUSH] = weight;
            }
        }
    }

    return HeuristicParameters(ec, dbl, dbh, cb, rb, hsw, bw, hswMap);
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " -b|-s|-r [-v] <filename>" << std::endl;
        return 1;
    }

    bool verbose = false;
    std::string mode = argv[1];
    std::string filename;

    if (argc == 4) {
        if (std::string(argv[2]) == "-v") {
            verbose = true;
            filename = argv[3];
        } else {
            std::cerr << "Invalid option: " << argv[2] << std::endl;
            return 1;
        }
    } else {
        filename = argv[2];
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return 1;
    }

    std::vector<HeuristicParameters> agentsParams;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (file.eof()) {
            std::cerr << "Not enough parameters in " << filename << std::endl;
            return 1;
        }
        agentsParams.push_back(readParams(file));
    }
    file.close();

    if (mode == "-b") {
        playGameBenchmark(NUM_GAMES, verbose, agentsParams);
    } else if (mode == "-s") {
        playGameShowdown(NUM_GAMES, verbose, agentsParams);
    } else if (mode == "-r") {
        playGameRandom(NUM_GAMES, verbose);
    } else if (mode == "-e") {
        evaluateParameterSets(50);
    } else {
        std::cerr << "Invalid option: " << mode << std::endl;
        return 1;
    }

    return 0;
}
