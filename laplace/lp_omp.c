#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
// MacOS
#include "/opt/homebrew/opt/libomp/include/omp.h"

#define ITER_MAX 3000 //number of max iterations
// #define TOL 0.001

// matrix to be solved
double **grid;

// auxiliary matrix
double **new_grid;

// size of each side of the grid
int size;

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
    sprintf(file_name, "grid_lp_omp.txt");

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

//libera a memória
void free_memory(){
  for (int i = 0; i < size; i++){
    free(grid[i]);
    free(new_grid[i]);
  }

  free(grid);
  free(new_grid);
}

void Jacobi(double** matrix, double** matrix_new, int size, int n_threads) {
  int i, j;
  int iter = 0;

  while (iter <= ITER_MAX){

    omp_set_num_threads(n_threads);
    #pragma omp parallel for collapse(2)
    for (i = 1; i < size - 1; i++) {
      for (j = 1; j < size - 1; j++) {
        matrix_new[i][j] = 0.25 * (matrix[i-1][j] + matrix[i+1][j] + matrix[i][j-1] + matrix[i][j+1]);
      }
    }

    #pragma omp parallel for collapse(2)
    for (i = 1; i < size - 1; i++) {
      for (j = 1; j < size - 1; j++) {
        matrix[i][j] = matrix_new[i][j];
      }
    }

    iter ++;
  }
}

int main(int argc, char *argv[]) {

    if(argc != 3){
        printf("Usage: ./lp_omp N T\n");
        printf("N: The size of each side of the domain (grid)\n");
        printf("T: Number of threads\n");
        exit(-1);
    }

    // //Number of iterations
    // int num_iter = atoi(argv[1]);
    
    // size of each side of the grid
    size = atoi(argv[1]);

    //Number of threads
    int nthreads = atoi(argv[2]);

    //aloccate and initialize grid
    allocate_memory();
    initialize_grid();

    // get the start time
    double exec_time = omp_get_wtime();

    //paralelizacao com openMP
    Jacobi (grid, new_grid, size, nthreads);

    // get the end time
    exec_time = omp_get_wtime() - exec_time;

    //save grid in a file 
    save_grid();

    printf("\nLaplace executed in %lf seconds with %d procs\n\n", exec_time, nthreads);

    //libera a memória
    free_memory();

    return 0;
}

