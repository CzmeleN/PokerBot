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
- `-b` - benchmark for MCTS heuristics (will benchmark all Agents in `params.bots`)
- `-s` - showdown (5 agents from the top of `params.bots` competing against each other)
- `-e` - evaluate 3 ^ 7 different heuristics (deprecated)

**Verbose flag `-v` will show you game states move by move**

**Required `params.bots` format:**
- Agent name
- Exploration constant
- Draw bias for low-value hands
- Draw bias for high-value hands
- Call bias multiplier (* hand value)
- Raise bias multiplier (* hand value)
- Hand strength weight (additional multiplier for hand value)
- Bankroll weight (how much influence does already bet cash have)
- Hand strength weights: `HIGH_CARD [val] PAIR [val] TWO_PAIR [val] THREE_KIND [val] STRAIGHT [val] FLUSH [val] FULL_HOUSE [val] FOUR_KIND [val] STRAIGHT_FLUSH [val] ROYAL_FLUSH [val]`

### Worth noting:
This is by no means a ready solution for gambling and I strongly advise against using it. Provided agents' parameters are just for an overview and further testing is required for them to become useful.
