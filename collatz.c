#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void printInfo(int, int, double, double, int, int, int, int);
int collatz(int);

int main(int argc, char **argv) {
  int lower_limit = atoi(argv[1]);
  int upper_limit = atoi(argv[2]);
  int grain_size = atoi(argv[3]);

  int world_rank;
  int world_size;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); /* Id of process */
  MPI_Comm_size(MPI_COMM_WORLD, &world_size); /* Number of processors */
  
  if (world_rank == 0) {
    /* Father process */

    int message[2] = {            /* Message to be sent to child process {stop, start_number */
                      0,          /* Boolean value to determine if child process should stop */
                      lower_limit /* Integer from which child process has to start calculating local minimum */
    };

    /* Send initial data to processes */
    double start_time = MPI_Wtime(); /* Begin performance tracking */

    for (int i = 1; i < world_size; i++) {
      MPI_Send(&message, 2, MPI_INT, i, 1, MPI_COMM_WORLD);
      message[1] += grain_size; /* Increase current_number */
    }

    int maximum_pair[2]; /* We'll hold information here about longest number: {integer, iterations} */

    /* Continue sending messages to child processes until we reach upper limit */
    while (message[1] < upper_limit) {
      MPI_Recv(&maximum_pair, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
      MPI_Send(&message, 2, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
      message[1] += grain_size;
    }

    /* End child processes and find maximum integer and iterations of our given interval */
    int max_iterations = 0; /* Global maximum iterations */
    int max_integer;        /* Global maximum integer */
    message[0] = 1;         /* Set stop to true */
    
    for (int j = 1; j < world_size; j++) {
      MPI_Recv(&maximum_pair, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

      /* Compare maximum of process to global maximum */
      if (max_iterations < maximum_pair[1]) {
        max_integer = maximum_pair[0];
        max_iterations = maximum_pair[1];
      }

      MPI_Send(&message, 2, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
    }

    double end_time = MPI_Wtime();

    printInfo(world_size, grain_size, start_time, end_time, lower_limit, upper_limit, max_integer, max_iterations);
  } else {
    /* Child process */
    int start_number;
    int end_number;
    int max_iterations = 0;  /* Local maximum iterations in this process */
    int max_integer;         /* Integer to which maximum belongs in the interval of this process */
    int stop = 0;            /* Boolean value to check if cycle should continue */
    int received_message[2]; /* Stores message received from main process: {stop, start_number} */
    int current_iterations;

    for (;;) {
      MPI_Recv(&received_message, 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

      /* Check if we should continue process */
      stop = received_message[0];
      if (stop) {
        break;
      }

      /* Find local minimum of current interval */
      start_number = received_message[1];
      end_number = start_number + grain_size - 1;

      /* Check if end number does not go above upper limit */
      if (end_number < upper_limit) {
        end_number = upper_limit;
      }

      /* Find maximum of interval and compare it to the maximum that process has found so far */
      for(int i = start_number; i < end_number; i++){
        current_iterations = collatz(i);

        if(max_iterations < current_iterations){
          max_iterations = current_iterations;
          max_integer = i;
        }
      }

      /* Generate message for father process */
      int maximum_pair[2] = {max_integer, max_iterations};
      MPI_Send(&maximum_pair, 2, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();

  return 0;
}

void printInfo(int world_size, int grain_size, double start_time, double end_time, int lower_limit, int upper_limit, int maxInteger, int maxIterations){
  printf("Cores: %3d, grain size: %5d, time: %f ms, interval [%d; %d], max [%d, %d]\n", world_size, grain_size, (end_time - start_time), lower_limit, upper_limit, maxInteger, maxIterations);
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