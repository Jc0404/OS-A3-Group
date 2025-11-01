# COMP SCI 3004 OS Assignment 3 Concurrency

- Authors: JiaZhi Chen (a1868575), Samuel Brown (a1833328)
- Group name: Pair 4

## Overview

This project implements the COMP2002 Assignment 3 requirement, a multi threaded merge sort using the POSIX Threads (pthreads) library.

The input array is recursively divided into halves; up to a user-defined cutoff depth, each half is processed in parallel by a new thread. Beyond this depth, sorting proceeds sequentially.

The parent thread joins both child threads before performing the merge step, ensuring correctness and preventing data races.

## Manifest

- `mergesort.c` — Contains the parallel recursion (`parallel_mergesort`) and argument builder (`buildArgs`). `merge` function to be invoked by sort functions to combine the resulting arrays. `my_mergesort` sequential recursive merge sort to be used as fallback for the parallel merge sort.
- `mergesort.h` — Shared declarations, including `struct argument`, `cutoff`, and the array globals.
- `test-mergesort.c` — Provided test file. Generates random inputs, times execution, and validates sorted output.
- `test-merge.c` — Simple test to verify the functionality of the basic merge function. Provides two sorted arrays to be merged.
- `test-my-mergesort.c` — Based on the concepts employed in the provided test file, generates a random array to be sorted by the non-parallel mergesort function. Reports execution times so results can be compared.
- `Makefile` — Builds the `test-mergesort`, `test-merge.c` and `test-my-mergesort.c` executables with the necessary pthread flags.
- `README.md` — This documentation.

## Building the project

```bash
cd comp2002-os-mergesort
make
```

This will produces the executable `./test-mergesort`.

```bash
cd comp2002-os-mergesort
make test-merge
```

This will produces the executable `./test-merge`. Verifies the operations of the merge function.

```bash
cd comp2002-os-mergesort
make test-my-mergesort
```

This will produces the executable `./test-my-mergesort`. Validates the functionality of the sequential, single threaded merge sort.

## Features and usage

- Parallel recursion with a configurable depth limit (`cutoff`).
- Graceful fallback to sequential processing if thread creation or argument allocation fails.
- Deterministic sorting; only performance varies with thread levels.

Run:

```bash
./test-mergesort <input_size> <cutoff_level> <seed>
# example:
./test-mergesort 1000000 3 1234
```

## Testing

- Directory layout

  - `test-mergesort.c` remains part of the submission set. Auxiliary sources live under `tests/`, and their executables are emitted to `tests/bin` to keep the delivery tree tidy.

- Merge routine check

  ```bash
  cd comp2002-os-mergesort
  make test-merge
  ./tests/bin/test-merge
  ```

  Expected output:

  ```
  Before merge:
  1 3 5 7 9 2 4 6 8 10
  After merge:
  1 2 3 4 5 6 7 8 9 10
  Result: correctly sorted
  ```

  Demonstrates that the `merge` function stitches the two halves into a single sorted range.

- Sequential baseline (`my_mergesort`)

  ```bash
  make test-my-mergesort
  ./tests/bin/test-my-mergesort 100 123
  ```

  Expected output:

  ```
  Testing my_mergesort with array size 100 and seed 123
  Array A: ... (unsorted sample)
  Array A: ... (sorted sample)
  Array is correctly sorted
  Sorting 100 elements took XX.XX milliseconds (YY.YY seconds).
  ```

  Confirms the sequential implementation sorts random data and reports timing.

- Parallel (`parallel_mergesort` + `buildArgs`)

  ```bash
  make test-parallel
  ./tests/bin/test-parallel-mergesort
  ```

  Expected output:

  ```
  === parallel_mergesort test ===
  [buildArgs]
    pthread_create_calls = 0, pthread_join_calls = 0
    array_sorted = YES
    fail_trigger = 0, pthread_create_failures = 0
    struct fields -> left=3 right=7 level=2 (expected 3,7,2)
    field_match = PASS
  [cutoff=0]
    pthread_create_calls = 0, pthread_join_calls = 0
    array_sorted = YES
    fail_trigger = 0, pthread_create_failures = 0
    expectation: pthread_create_calls == 0 -> PASS
  [cutoff=2]
    pthread_create_calls = 6, pthread_join_calls = 6
    array_sorted = YES
    fail_trigger = 0, pthread_create_failures = 0
    expectation: pthread_create_calls >= 2 -> PASS
  [create failure]
    pthread_create_calls = 4, pthread_join_calls = 3
    array_sorted = YES
    fail_trigger = 1, pthread_create_failures = 1
    expectation: pthread_create_failures >= 1 -> PASS
  Summary: all scenarios completed successfully.
  ```

  Highlights the threading behaviour, cutoff control, and fallback logic.

- Final test

  ```bash
  ./test-mergesort 1000 0 42
  ./test-mergesort 1000 2 42
  ```

  Expected behaviour: `cutoff=0` prints the serial timing, while `cutoff=2` shows the same array sorted more quickly, demonstrating the required parallel speedup.

- Performance experiment

  Example run (n = 100,000,000, seed = 1234):

  Expected output:

  ```
  cutoff=0  -> Sorting 100000000 elements took 26.22 seconds.
  cutoff=1  -> Sorting 100000000 elements took 13.49 seconds.
  cutoff=2  -> Sorting 100000000 elements took 9.94 seconds.
  cutoff=3  -> Sorting 100000000 elements took 6.30 seconds.
  cutoff=4  -> Sorting 100000000 elements took 6.33 seconds.
  cutoff=5  -> Sorting 100000000 elements took 6.12 seconds.
  cutoff=6  -> Sorting 100000000 elements took 6.06 seconds.
  cutoff=7  -> Sorting 100000000 elements took 6.07 seconds.
  cutoff=8  -> Sorting 100000000 elements took 6.16 seconds.
  cutoff=9  -> Sorting 100000000 elements took 6.71 seconds.
  ```

  Shows a speedup greater than 2× when moving from cutoff 0 to cutoff 3, with diminishing gains once the recursion grows deeper than level 5.

## Known Bugs

- None observed in local testing; continue monitoring large-input runs for scheduling variance.

## Reflection and Self Assessment

- Key design choice: worker threads free their argument blocks (level>0) while the root context leaves ownership to the caller; this avoids double-free of the initial struct yet still prevents leaks in spawned threads. Joining before merging guarantees sorted halves are finalised, eliminating race conditions, specifically preventing an Order Violation Bug.
- Remaining risk: overall speedup depends on the concrete implementations of `merge`/`my_mergesort` and on runtime scheduling or available CPU cores.
- Lack of Locks and Semaphores: The code could be more complex, in order to demonstrate the utility of locks and semaphores. However, due to the nature and scope of the task it was deemed more suitable to not employ these methods. Added unnecessary complexity comes with a greater potential for errors, especially so when dealing with mutual exclusion. Thus the approach focused around utilising pthread_join was selected, which enables us to entirely avoid a Deadlock problem. The hierarchical approach taken also removes the potential for a Circular Wait to occur.
- Reflection: This code effectively demonstrates the key advantages of a multithreaded approach, by directly contrasting it against a single threaded, sequential array sort. Multithreading can provide several challenges to overcome, such as being mindful of critical sections and race conditions. However, depending on the desired application, it can be a truly invaluable tool in ensuring efficiency and timely operations.

## Sources Used

- Assignment 3 Concurrency handout.
- OSTEP “Intro to Threads” and “Threads API” chapters (sections 27.1–27.2).
- `man pthread_create`, `man pthread_join`, standard C library references.
