#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void startInfo(int, int, int, int);
void endInfo(double, double, int, int, int, int);
int collatz(int);

int main(int argc, char **argv) {
  int lower_limit = atoi(argv[1]);
  int upper_limit = atoi(argv[2]);
  int grain_size = atoi(argv[3]);
  
  int world_rank;
  int world_size;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); /* id of process */
  MPI_Comm_size(MPI_COMM_WORLD, &world_size); /* number of processors */
  
  if (world_rank == 0) {
    /* father process */
    startInfo(lower_limit, upper_limit, grain_size, world_size);
    int maxIterations = 0; /* Global maximum iterations */
    int maxInteger;        /* Global maximum integer */
    int current_number = lower_limit;
    double start_time = MPI_Wtime();

    int numberLeap = (upper_limit - lower_limit + 1) / (world_size - 1) - 1;

    for (int i = 1; i < world_size; i++) {
      MPI_Send(&current_number, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(&numberLeap, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
      current_number += numberLeap + 1;
    }

    int received_iterations = 0; /* iterations received from child process */
    int received_integer;
    
    for (int j = 1; j < world_size; j++) {
      MPI_Recv(&received_iterations, 1, MPI_INT, j, 3, MPI_COMM_WORLD, &status);
      MPI_Recv(&received_integer, 1, MPI_INT, j, 4, MPI_COMM_WORLD, &status);

      if (maxIterations < received_iterations) {
        maxIterations = received_iterations;
        maxInteger = received_integer;
      }
    }
    
    double end_time = MPI_Wtime();

    endInfo(start_time, end_time, lower_limit, upper_limit, maxInteger, maxIterations);
  }
  else {
  /* child process */
    int start_number;
    MPI_Recv(&start_number, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    int numberLeap;
    MPI_Recv(&numberLeap, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
    int end_number = start_number + numberLeap;
    //printf("I am %d and I need to process %d numbers from %d to %d!\n", world_rank, (numberLeap + 1), start_number, end_number);

    int max_iterations = 0; /* Local maximum iterations in this process */
    int max_integer;        /* Integer to which maximum belongs in the interval of this process */
    int currentIterations;

    for(int i = start_number; i < end_number; i++){
      currentIterations = collatz(i);

      if(max_iterations < currentIterations){
        max_iterations = currentIterations;
        max_integer = i;
      }
    }

    printf("I am %d and longest number was %d with %d iterations!\n", world_rank, max_integer, max_iterations);
    MPI_Send(&max_iterations, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    MPI_Send(&max_integer, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}

void startInfo(int lower_limit, int upper_limit, int grain_size, int world_size){
  printf("Interval from %d to %d, grain size %d, processor count: %d\n", lower_limit, upper_limit, grain_size, world_size);
}

void endInfo(double start_time, double end_time, int lower_limit, int upper_limit, int maxInteger, int maxIterations){
  printf("In interval from \x1B[31m%d\x1B[0m to \x1B[31m%d\x1B[0m number \x1B[31m%d\x1B[0m took longest with \x1B[31m%d\x1B[0m iterations.\nTime elapsed: \x1B[31m%f\x1B[0m ms\n", lower_limit, upper_limit, maxInteger, maxIterations, (end_time - start_time));
}

int collatz(int number){ 
  int currentNumber = number;
  int currentIterations = 0;

  while (currentNumber != 1) {
    if (currentNumber % 2 == 0) {
      currentNumber /= 2;
    } else {
      currentNumber = 3 * currentNumber + 1;
    }

    currentIterations++;
  }

  return currentIterations;
}