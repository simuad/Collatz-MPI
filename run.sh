#!/bin/bash
mpicc -o collatz collatz.c
mpirun -np 4 collatz 1 10000000 32768