# PokerBot

Bot playing five-card draw poker game made as a project for Artificial Intelligence

## Usage:

**Compile with:**

```sh
$ make
```

**Run:**

```sh
$ ./bot [mode flag] [verbose flag (optional)] <filename (with agents' parameters)>
```

**Modes:**
- `-r` - completely random 10000 games
- `-b` - benchmark for MCTS heuristics (will benchmark all Agents in `*.params` against a safe, normal, aggressive and aggressive-raising simple bots)
- `-t` - all bots from `*params` compete against each other (in a tournament-like ladder, powers of 5 are preferable)
- `-e` - evaluate 3 ^ 7 different heuristics (deprecated)

**Verbose flag `-v` will show you game states move by move**

**Required `params.bots` format:**
- Agent name (won't be used, needed for formatting)
- Exploration constant for MCTS
- Draw bias for low-value hands (pair and lower)
- Draw bias for high-value hands (three and higher)
- Call bias multiplier (* hand value - likelihood of calling instead of passing)
- Raise bias multiplier (* hand value - likelihood of raising instead of calling)
- Hand strength weight (additional multiplier for hand value)
- Bankroll weight (how much influence does already bet cash have)
- Hand strength weights: `HIGH_CARD [val] PAIR [val] TWO_PAIR [val] THREE_KIND [val] STRAIGHT [val] FLUSH [val] FULL_HOUSE [val] FOUR_KIND [val] STRAIGHT_FLUSH [val] ROYAL_FLUSH [val]`

### Worth noting:
This is by no means a ready solution for gambling and I strongly advise against using it. Provided agents' parameters are just for an overview and further testing is required for them to become useful.
