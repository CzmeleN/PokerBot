#include <bits/stdc++.h>

static const int NUM_GAMES = 100;
static const int BET_SIZE = 10;
static const int MCTS_ITERATIONS = 1000;

enum Suit { HEARTS, DIAMONDS, CLUBS, SPADES };
enum Rank { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };
enum Phase { EXCHANGE, BIDDING, SHOWDOWN };

struct Card {
    Suit suit;
    Rank rank;
    Card(Suit s, Rank r) : suit(s), rank(r) {}
};

enum ActionType {
    DRAW,
    CALL,
    RAISE,
    FOLD
};

enum HandValue { HIGH_CARD, PAIR, TWO_PAIR, THREE_KIND, STRAIGHT, FLUSH, FULL_HOUSE, FOUR_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH };

struct Move {
    ActionType action;
    int amount; //kwota zakładu
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

//HandValue evaluateHandStrength() const {
//        std::vector<Suit> suits;
//        std::vector<Rank> ranks;
//        std::set<Suit> suits_set;
//        std::set<Rank> ranks_set;
//
//        for (const auto& card : cards) {
//            suits.push_back(card.suit);
//            ranks.push_back(card.rank);
//            suits_set.insert(card.suit);
//            ranks_set.insert(card.rank);
//        }
//
//        bool straight = ranks_set.size() == 5 ? is_straight(ranks) : false;
//
//        if (suits_set.size() == 1 && straight) {
//            if (ranks_set.find(ACE) != ranks_set.end() && ranks_set.find(TEN) != ranks_set.end()) {
//                return ROYAL_FLUSH;
//            }
//            return STRAIGHT_FLUSH;
//        }
//        if (ranks_set.size() == 2) {
//            int count = std::count(ranks.begin(), ranks.end(), *ranks_set.begin());
//
//            if (count == 4 || count == 1) {
//                return FOUR_KIND;
//            }
//            return FULL_HOUSE;
//        }
//        if (suits_set.size() == 1) {
//            return FLUSH;
//        }
//        if (straight) {
//            return STRAIGHT;
//        }
//        if (ranks_set.size() == 3) {
//            int count = std::count(ranks.begin(), ranks.end(), *ranks_set.begin());
//            
//            if (count == 3 || count == 1) {
//                return THREE_KIND;
//            }
//            return TWO_PAIR;
//        }
//        if (ranks_set.size() == 4) {
//            return PAIR;
//        }
//
//        return HIGH_CARD; 
//    }
//
//private:
//    bool is_straight(const std::vector<Rank>& ranks) const {
//        std::vector<Rank> sorted = ranks;
//        
//        std::sort(sorted.begin(), sorted.end());
//
//        if (sorted[0] == TWO && sorted[1] == THREE && sorted[2] == FOUR && sorted[3] == FIVE && sorted[4] == ACE) {
//            return true;
//        }
//
//        for (short i = 0; i < 4; i++) {
//            if (sorted[i + 1] - sorted[i] != 1) {
//                return false;
//            }
//        }
//
//        return true;
//    }


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

    GameState(int numPlayers) : currentPlayer(0), lastRaiser(-1), pot(0), currentBet(BET_SIZE), phase(EXCHANGE) {
        playersHands.resize(numPlayers);
        bets.resize(numPlayers, BET_SIZE);
        activePlayers.resize(numPlayers, true);
        wallets.resize(numPlayers, NUM_GAMES * BET_SIZE);
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

            // Przejście do fazy BIDDING, jeśli obecny gracz jest ostatnim graczem
            if (currentPlayer == playersHands.size() - 1) {
                phase = BIDDING;
                return;
            }
        } else if (move.action == CALL || (move.action == RAISE && currentBet == 100)) {
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
                    winners = {static_cast<int>(i)};
                } else if (currentHand == bestHand) {
                    winners.push_back(static_cast<int>(i));
                }
            }
        }
        return winners;
    }


    //std::vector<int> getResult() const {
    //    int bestHandStrength = 0;
    //    std::vector<int> winners;
    //    for (size_t i = 0; i < playersHands.size(); ++i) {
    //        if (activePlayers[i]) {
    //            int handStrength = playersHands[i].evaluateHandStrength();
    //            if (handStrength > bestHandStrength) {
    //                bestHandStrength = handStrength;
    //                winners.clear();
    //                winners.push_back(i);
    //            } else if (handStrength == bestHandStrength) {
    //                winners.push_back(i);
    //            }
    //        }
    //    }
    //    return winners;
    //}

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
    int wins, visits;
    Move move; 

    Node(GameState s, Node* p = nullptr, Move m = Move()) : state(s), parent(p), move(m), wins(0), visits(0) {}
};

struct HeuristicParameters {
    double explorationConstant;
    double drawBias;
    double callBiasBase;
    double raiseBiasBase;
    double handStrengthWeight;
    double bankrollWeight;
    std::map<HandValue, double> handStrengthWeights;

    HeuristicParameters(double ec, double db, double cb, double rb, double hsw, double bw, std::map<HandValue, double> hswMap)
        : explorationConstant(ec), drawBias(db), callBiasBase(cb), raiseBiasBase(rb), 
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
    Node* root;
    HeuristicParameters params;
    std::mt19937 generator;
public:
    MCTS(GameState rootState, HeuristicParameters hp) : params(hp), root(new Node(rootState, nullptr, Move())), generator(std::random_device{}()) {}

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
        while (!state.isTerminal()) {
            auto moves = state.getPossibleMoves();
            std::vector<double> moveProbabilities(moves.size(), 1.0);
            for (size_t i = 0; i < moves.size(); ++i) {
                double probability = 1.0;
                HandEvaluation handEvaluation = state.playersHands[state.getCurrentPlayer()].evaluateHandStrength();
                HandValue handValue = handEvaluation.value;

                if (moves[i].action == DRAW) {
                    probability *= params.drawBias;
                } else if (moves[i].action == CALL) {
                    probability *= params.callBiasBase * (1.0 + params.handStrengthWeights.at(handValue));
                } else if (moves[i].action == RAISE) {
                    probability *= params.raiseBiasBase * (1.0 + params.handStrengthWeights.at(handValue));
                }

                // Dodatkowe oceny heurystyczne
                if (state.phase != EXCHANGE) {
                    probability *= (1.0 + params.handStrengthWeight * handEvaluation.value);
                }

                double bankrollFactor = 1.0 + params.bankrollWeight * state.bets[state.getCurrentPlayer()];
                probability *= bankrollFactor;

                moveProbabilities[i] = probability;
            }

            std::discrete_distribution<int> dist(moveProbabilities.begin(), moveProbabilities.end());
            int chosenMoveIndex = dist(generator);
            state.applyMove(moves[chosenMoveIndex]);
        }
        return state.getResult();
    }


    void backpropagate(Node* node, const std::vector<int>& result) {
        while (node != nullptr) {
            node->visits++;
            if (std::find(result.begin(), result.end(), node->state.getCurrentPlayer()) != result.end()) {
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

void playGameRandom(int numGames, bool verbose) {
    int numPlayers = 5;
    numGames *= 10;

    std::vector<int> wallets(numPlayers, 10000);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int game = 0; game < numGames; ++game) {
        GameState initialState(numPlayers);
        initialState.wallets = wallets;

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
        wallets = initialState.wallets;

        if (verbose) initialState.print();

        auto winners = initialState.getResult();
        std::cout << "Game " << game + 1 << " Winners: ";
        for (int winner : winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < numPlayers; ++i) {
            std::cout << "Player " << i << " wallet: " << wallets[i] << std::endl;
        }
    }
}

void playGameBenchmark(int numGames, bool verbose) {
    std::map<HandValue, double> handStrengthWeights = {
        {HIGH_CARD, 0.1}, {PAIR, 0.2}, {TWO_PAIR, 0.3}, {THREE_KIND, 0.4},
        {STRAIGHT, 0.5}, {FLUSH, 0.6}, {FULL_HOUSE, 0.7}, {FOUR_KIND, 0.8},
        {STRAIGHT_FLUSH, 0.9}, {ROYAL_FLUSH, 1.0}
    };

    HeuristicParameters params(1.41, 1.0, 1.0, 1.0, 0.1, 0.1, handStrengthWeights);
    int numPlayers = 5;

    std::vector<int> wallets(numPlayers, 1000);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int game = 0; game < numGames; ++game) {
        GameState initialState(numPlayers);
        initialState.wallets = wallets;

        Deck deck;
        for (auto& hand : initialState.playersHands) {
            for (int i = 0; i < 5; ++i) {
                hand.addCard(deck.drawCard());
            }
        }
        initialState.deck = deck;

        MCTS mcts(initialState, params);

        while (!initialState.isTerminal()) {
            if (initialState.getCurrentPlayer() == 0) {
                MCTS mcts(initialState, params);
                mcts.runSearch(MCTS_ITERATIONS);
                Move bestMove = mcts.getBestMove();
                initialState.applyMove(bestMove);
            } else {
                auto possibleMoves = initialState.getPossibleMoves();
                std::uniform_int_distribution<int> dist(0, possibleMoves.size() - 1);
                int chosenMoveIndex = dist(gen);
                initialState.applyMove(possibleMoves[chosenMoveIndex]);
            }
        }

        initialState.updateWallets();
        wallets = initialState.wallets;

        if (verbose) initialState.print();

        auto winners = initialState.getResult();
        std::cout << "Game " << game + 1 << " Winners: ";
        for (int winner : winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < numPlayers; ++i) {
            std::cout << "Player " << i << " wallet: " << wallets[i] << std::endl;
        }
    }
}

void playGameShowdown(int numGames, bool verbose) {
    std::map<HandValue, double> handStrengthWeights = {
        {HIGH_CARD, 0.1}, {PAIR, 0.2}, {TWO_PAIR, 0.3}, {THREE_KIND, 0.4},
        {STRAIGHT, 0.5}, {FLUSH, 0.6}, {FULL_HOUSE, 0.7}, {FOUR_KIND, 0.8},
        {STRAIGHT_FLUSH, 0.9}, {ROYAL_FLUSH, 1.0}
    };

    HeuristicParameters params(1.41, 1.0, 1.0, 1.0, 0.1, 0.1, handStrengthWeights);
    int numPlayers = 5;

    std::vector<int> wallets(numPlayers, 1000);
    for (int game = 0; game < numGames; ++game) {
        GameState initialState(numPlayers);
        initialState.wallets = wallets;

        Deck deck;
        for (auto& hand : initialState.playersHands) {
            for (int i = 0; i < 5; ++i) {
                hand.addCard(deck.drawCard());
            }
        }
        initialState.deck = deck;

        std::vector<MCTS> mctsBots;
        for (int i = 0; i < numPlayers; ++i) {
            GameState botState = initialState; // Tworzenie kopii stanu dla każdego bota
            mctsBots.emplace_back(botState, params);
        }

        while (!initialState.isTerminal()) {
            int currentPlayer = initialState.getCurrentPlayer();
            GameState simulationState = initialState; // Tworzenie kopii stanu dla symulacji
            mctsBots[currentPlayer] = MCTS(simulationState, params);
            mctsBots[currentPlayer].runSearch(MCTS_ITERATIONS);
            Move bestMove = mctsBots[currentPlayer].getBestMove();
            initialState.applyMove(bestMove);

            // Debugging output
        }

        initialState.updateWallets();
        wallets = initialState.wallets;
        
        if (verbose) initialState.print();


        auto winners = initialState.getResult();
        std::cout << "Game " << game + 1 << " Winners: ";
        for (int winner : winners) {
            std::cout << winner << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < numPlayers; ++i) {
            std::cout << "Player " << i << " wallet: " << wallets[i] << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " -b|-s [-v]" << std::endl;
        return 1;
    }

    bool verbose = false;
    if (argc == 3) {
        if (std::string(argv[2]) == "-v") {
            verbose = true;
        } else {
            std::cerr << "Invalid option: " << argv[2] << std::endl;
            return 1;
        }
    }

    if (std::string(argv[1]) == "-b") {
        playGameBenchmark(NUM_GAMES, verbose);
    } else if (std::string(argv[1]) == "-s") {
        playGameShowdown(NUM_GAMES, verbose);
    } else if (std::string(argv[1]) == "-r") {
        playGameRandom(NUM_GAMES, verbose);
    } else {
        std::cerr << "Invalid option: " << argv[1] << std::endl;
        return 1;
    }

    return 0;
}
