#!/bin/bash
mpicc -o collatz collatz.c
mpirun -np 4 collatz 1 10000 4