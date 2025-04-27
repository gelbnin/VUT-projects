# Skibus Simulation Project (proj2)

## Description

This project simulates a skibus system using multiple processes in C. It involves a main process, a single skibus process, and multiple skier processes[cite: 1, 9]. Skiers start, wait for a random time (simulating breakfast), go to a randomly assigned bus stop, and wait for the skibus[cite: 2, 17, 21]. The skibus travels sequentially through all designated stops[cite: 4]. When the bus arrives at a stop, waiting skiers board up to the bus's capacity[cite: 3]. After visiting all stops, the bus transports the skiers to a final destination (near a ski lift) where they disembark[cite: 4, 22]. The bus repeats this cycle if there are more skiers waiting or potentially arriving at stops[cite: 5]. The simulation uses shared memory and semaphores for inter-process communication and synchronization[cite: 10, 11, 12]. All actions are logged to an output file `proj2.out`[cite: 7].

## Prerequisites

* GCC compiler.
* Make utility.
* A POSIX-compliant system.

## Usage

* L: Total number of skiers (L < 20000).
* Z: Number of bus stops ($0 < Z \le 10$).
* K: Capacity of the skibus ($10 \le K \le 100$).
* TL: Maximum time (in microseconds) a skier waits before going to the bus stop ($0 \le TL \le 10000$). Represents time after "breakfast".
* TB: Maximum time (in microseconds) for the bus to travel between stops ($0 \le TB \le 1000$).

Run the compiled program from the command line:

```bash
make
./proj2 L Z K TL TB
```

## Evaluation

15/15