# Longest Collatz Sequence with MPI

C program with MPI to to find longest Collatz sequence in given interval, which runs on VU cluster.

## Compilation

```bash
mpicc -o collatz collatz.c
```

## How to run

```bash
mpirun -np cores collatz start end grain
```
Where:
* cores - core count
* start - start of interval
* end - end of interval
* grain - grain size