# PokerBot
Bot playing five-card draw poker game made as a project for Artificial Intelligence

## Usage:

**Compile with:**

$ g++ -O2 -o bot.o bot.cpp
or
$ g++ -Os -o bot.o bot.cpp

**Run ./bot.o [mode flag] [verbose flag (optional)]**

**modes:**
-r - completely random 10000 games
-b - benchmark for MCTS heurestics (will benchmark all Agents in params.bots)
-s - showdown (5 agents from the top of params.bots competing against each other)

**verbose flag -v will show you game state move by move**

**params.bots required format:**
 - Agent name
 - exploration constant
 - draw bias for low-value hands
 - draw bias for high-value hands
 - call bias multiplier (* hand value)
 - raise bias multiplier (* hand value)
 - hand strenght weight (additional multiplier for hand value)
 - bankroll weight (how much influence does already bet cash have)
 - hand strength weights HIGH_CARD [val] PAIR [val] TWO_PAIR [val] THREE_KIND [val] STRAIGHT [val] FLUSH [val] FULL_HOUSE [val] FOUR_KIND [val] STRAIGHT_FLUSH [val] ROYAL_FLUSH [val]

### Worth noting:
This is by no means a ready solution for gambling and I strongly advise against using it.
Provided agents parameters are just for an overview and further testing is required for them to become useful.
