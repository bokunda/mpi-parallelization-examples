/**
 * Name: Cancer simulation MPI program
 * Author: Bojan Piskulić
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define CANCER 0
#define MIN_CANCER_HEALTH 0
#define MAX_CANCER_HEALTH 100
#define CANCER_PROB 2

#define MEDECINE 1
#define MIN_MEDECINE_HEALTH 1
#define MAX_MEDECINE_HEALTH 20
#define MEDECINE_PROB 18

#define TISSUE 2
#define MIN_TISSUE_HEALTH 0
#define MAX_TISSUE_HEALTH 10000
#define TISSUE_PROB 80

#define PRINT_TYPE 1
#define PRINT_HEALTH 2

#define M 10
#define N 10
#define STEPS 10

typedef struct Cell {
    int type;
    int health;
} cell;

// Manually implemented MPI_MAX function
void myMax(int *invector, int *outvector, int *len, MPI_Datatype type)
{
    *outvector = *invector > *outvector ? *invector : *outvector;
}

// Allocates a new matrix m x n size
cell** allocateMatrix(int m, int n)
{
    cell** matrix;
    matrix = (cell**)malloc(m * sizeof(cell*));
    for (int i = 0; i < m; i++) matrix[i] = (cell*)malloc(n * sizeof(cell));
    return matrix;
}

// Generatetes cell value (TISSUE, MEDECINE, CANCER) with health
cell generateCellValue()
{
    cell newCell;
    int prob = rand() % 100 + 1;

    if (prob <= TISSUE_PROB)
    {
        newCell.type = TISSUE;
        newCell.health = rand() % MAX_TISSUE_HEALTH + MIN_TISSUE_HEALTH;
    }
    else if (prob <= TISSUE_PROB + MEDECINE_PROB)
    {
        newCell.type = MEDECINE;
        newCell.health = rand() % MAX_MEDECINE_HEALTH + MIN_MEDECINE_HEALTH;
    }
    else
    {
        newCell.type = CANCER;
        newCell.health = rand() % MAX_CANCER_HEALTH + MIN_CANCER_HEALTH;
    }
    return newCell;
}

// Generates cell values for the whole matrix
cell** setMatrix(cell** matrix, int m, int n)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = generateCellValue();
        }
    }
    return matrix;
}

// Prints matrix data based on params
void printMatrix(cell** matrix, int m, int n, int printOption)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (printOption == PRINT_TYPE)
            {
                printf("%d ", matrix[i][j].type);
            }
            else
            {
                printf("%d ", matrix[i][j].health);
            }
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    int rank, size;

    // MPI logic starts here
    MPI_Init(&argc, &argv);

    // Get process rank and total number of processes (size)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Create and commit cell structure
    MPI_Datatype mycelltype;

    int count = 1;
    int lengths[1] = { 1 };
    MPI_Aint displacements[1] = { 0 };
    MPI_Datatype types[1] = { MPI_INT };

    MPI_Type_create_struct(count, lengths, displacements, types, &mycelltype);
    MPI_Type_commit(&mycelltype);

    // Register reduction operation
    MPI_Op MPI_MY_MAX;
    MPI_Op_create( (MPI_User_function *) myMax, 1, &MPI_MY_MAX);

    // Data prepare
    cell **matrix;

    if (rank == 0)
    {
        // Allocate matrix
        matrix = allocateMatrix(M, N);
        // Set matrix
        matrix = setMatrix(matrix, M, N);
        printMatrix(matrix, M, N, PRINT_TYPE);
    }

    // TODO: Sync matrix
    // TODO: Define submatrix for each process
    
    // Simulate
    // TODO: Loop through steps
    // TODO: Do Work - Simulate
    // TODO: Sync corner rows
    
    // Find strongest
    // TODO: Find local strongest
    // TODO: Find global strongest with custom redustion

    // Cleanup and finish
    MPI_Op_free(&MPI_MY_MAX);
    MPI_Finalize();

    return 0;
}