// Nao funcionou no mac pq ele nao suporta o uso de barreiras nativamente. Porem o algoritmo está correto 


/*
    This program solves Laplace's equation on a regular 2D grid using simple Jacobi iteration with parallel computing using pthread 

    The stencil calculation stops when  ( iter > ITER_MAX )
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define ITER_MAX 3000 // number of maximum iterations

int numThreads;
pthread_barrier_t barrier;

// matrix to be solved
double **grid;

// auxiliary matrix
double **new_grid;


// size of each side of the grid
int size;

// return the maximum value
double max(double a, double b){
    if(a > b)
        return a;
    return b;
}

// return the absolute value of a number
double absolute(double num){
    if(num < 0)
        return -1.0 * num;
    return num;
}

// allocate memory for the grid
void allocate_memory(){
    grid = (double **) malloc(size * sizeof(double *));
    new_grid = (double **) malloc(size * sizeof(double *));

    for(int i = 0; i < size; i++){
        grid[i] = (double *) malloc(size * sizeof(double));
        new_grid[i] = (double *) malloc(size * sizeof(double));
    }
}

// initialize the grid
void initialize_grid(){
    // seed for random generator
    srand(10);

    int linf = size / 2;
    int lsup = linf + size / 10;
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // inicializa região de calor no centro do grid
            if ( i>=linf && i < lsup && j>=linf && j<lsup)
                grid[i][j] = 100;
            else
               grid[i][j] = 0;
            new_grid[i][j] = 0.0;
        }
    }
}

// save the grid in a file
void save_grid(){

    char file_name[30];
    sprintf(file_name, "grid_laplace.txt");

    // save the result
    FILE *file;
    file = fopen(file_name, "w");

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            fprintf(file, "%lf ", grid[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void *LaplacePThread(void *args){
    int iter = 0;
    int waitReturn, counter, time;
    int id = *(int *) args;
    int chunk = size / numThreads;
    int start = id * chunk;
    int end = start + chunk;
    
    if (id == 0)
        start = 1;
    else
        start = chunk * id;

    if (id == numThreads - 1)
        end = size - 1;
    else
        end = chunk * (id + 1);

    while (iter <= ITER_MAX)
    {
        // calculates the Laplace equation to determine each cell's next value
        // kernel 1
        for (int i = start; i < end; i++)
        {
            for (int j = 1; j < size - 1; j++)
            {
                new_grid[i][j] = 0.25 * (grid[i][j + 1] + grid[i][j - 1] +
                                         grid[i - 1][j] + grid[i + 1][j]);
            }
        }

        pthread_barrier_wait(&barrier);

        // copy the next values into the working array for the next iteration
        // kernel 2
        for (int i = start; i < end; i++)
        {
            for (int j = 1; j < size - 1; j++)
            {
                grid[i][j] = new_grid[i][j];
            }
        }

        pthread_barrier_wait(&barrier);

        iter++;
    }
    return NULL;
}

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Usage: ./laplace_seq N T\n");
        printf("N: The size of each side of the domain (grid)\n");
        printf("T: number of threads\n");

        exit(-1);
    }

    // variables to measure execution time
    struct timeval time_start;
    struct timeval time_end;

    size = atoi(argv[1]);
    numThreads = atoi(argv[2]);
    
    if (pthread_barrier_init(&barrier, NULL, numThreads))
		printf("Impossible to create barrier");

    // allocate memory to the grid (matrix)
    allocate_memory();

    // set grid initial conditions
    initialize_grid();

    // get the start time
    gettimeofday(&time_start, NULL);

    // create an array of p_threads
    pthread_t threads[numThreads];

    // store each thread ID
    int thread[numThreads];

    //create the threads
    for(int i = 0; i < numThreads; i++){
        thread[i] = i;
        pthread_create(&threads[i], NULL, LaplacePThread, (void *) &thread[i]);
    }

    //wait each thread to finish 
    for (int i = 0; i < numThreads; i++)
        pthread_join(threads[i], NULL);

    // get the end time
    gettimeofday(&time_end, NULL);

    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    //save the final grid in file
    save_grid();

    printf("Kernel executed in %lf seconds with %d threads\n\n", exec_time, numThreads);

    return 0;
}
