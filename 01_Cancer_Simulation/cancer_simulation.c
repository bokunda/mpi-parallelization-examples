/**
 * Name: Cancer simulation MPI program
 * Author: Bojan Piskulić
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#define CANCER 0
#define MIN_CANCER_HEALTH 0
#define MAX_CANCER_HEALTH 101
#define CANCER_PROB 2

#define MEDECINE 1
#define MIN_MEDECINE_HEALTH 1
#define MAX_MEDECINE_HEALTH 20
#define MEDECINE_PROB 18

#define TISSUE 2
#define MIN_TISSUE_HEALTH 0
#define MAX_TISSUE_HEALTH 10001
#define TISSUE_PROB 80

#define PRINT_TYPE 1
#define PRINT_HEALTH 2
#define PRINT_ALL 3

#define M 10
#define N 10
#define STEPS 1000

typedef struct Cell {
    int type;
    int health;
} cell;

// Manually implemented MPI_MAX function
void myMax(int *invector, int *outvector, int *len, MPI_Datatype type)
{
    *outvector = *invector > *outvector ? *invector : *outvector;
}

// Allocate array
cell* allocateArray(int n)
{
    return (cell*)malloc(n * sizeof(cell));
}

// Allocates a new matrix m x n size
cell** allocateMatrix(int m, int n)
{
    cell** matrix;
    matrix = (cell**)malloc(m * sizeof(cell*));
    for (int i = 0; i < m; i++) matrix[i] = allocateArray(n);
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
            else if (printOption == PRINT_HEALTH)
            {
                printf("%d ", matrix[i][j].health);
            }
            else
            {
                printf("[%d]-%d ", matrix[i][j].type, matrix[i][j].health);
            }
        }
        printf("\n");
    }
}

// Check if compared cell is a tissue or medecine
int tryApplyCancerOrMedicine(cell **matrix, int i, int j, cell value)
{
    int hasCancerNeighbour = 0;

    if (matrix[i][j].type == TISSUE && value.type == CANCER)
    {
        matrix[i][j].health -= value.health;

        if (matrix[i][j].health <= 0)
        {
            matrix[i][j].type = CANCER;
            matrix[i][j].health = rand() % 101;
        }
        hasCancerNeighbour = 1;
    }

    if (matrix[i][j].type == CANCER && value.type == MEDECINE)
    {
        matrix[i][j].health -= value.health;
        
        if (matrix[i][j].health <= 0)
        {
            matrix[i][j].type = TISSUE;
            matrix[i][j].health = rand() % MAX_TISSUE_HEALTH + MIN_TISSUE_HEALTH;
        }
        else
        {
            hasCancerNeighbour = 1;
        }

    }

    return hasCancerNeighbour;
}

// Main function where we are simulating cells behaviour
void doWork(cell **matrix, int startM, int endM, int n)
{
    int i, j;
    for (i = startM; i <= endM; i++)
    {
        int hasCancerNeighbour = 0;
        for (j = 0; j < n; j++)
        {
            // check left
            if (j > 0) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i][j-1]);
            // check right
            if (j < n) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i][j+1]);
            // check top
            if (i > startM) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i-1][j]);
            // check bottom
            if (i < endM) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i+1][j]);
            // check top-left
            if (j > 0 && i > startM) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i-1][j-1]);
            // check top-right
            if (j < n && i > startM) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i-1][j+1]);
            // check bottom-left
            if (j > 0 && i < endM) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i+1][j-1]);
            // check bottom-right
            if (j < n && i < endM) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, matrix[i+1][j+1]);

            // convert cell into a tissue if there cell is surrounded with tissues
            if (hasCancerNeighbour == 0 && matrix[i][j].type == MEDECINE)
            {
                matrix[i][j].type = TISSUE;
                matrix[i][j].health = rand() % MAX_TISSUE_HEALTH + MIN_TISSUE_HEALTH;
            }
        }
    }
}

// Handles edge cases, similar like doWork
void doWorkEdge(cell **matrix, cell *rowToCompare, int n, int i)
{
    int j;
    int hasCancerNeighbour = 0;
    for (j = 0; j < n; j++)
    {
        // default check
        hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, rowToCompare[j]);

        // check edge-left
        if (j > 0) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, rowToCompare[j-1]);
        // check edge-right
        if (j < n) hasCancerNeighbour += tryApplyCancerOrMedicine(matrix, i, j, rowToCompare[j+1]);

        if (hasCancerNeighbour == 0 && matrix[i][j].type == MEDECINE)
        {
            matrix[i][j].type = TISSUE;
            matrix[i][j].health = rand() % MAX_TISSUE_HEALTH + MIN_TISSUE_HEALTH;
        }
    }
}

// Finds max cell health
cell findLocalStrongestCancerCell(cell **matrix, int startM, int endM, int n)
{
    int i, j;
    cell strongest;
    strongest.type = CANCER;
    strongest.health = -1;

    for (i = startM; i <= endM; i++)
    {
        for (j = 0; j < n; j++)
        {
            if (matrix[i][j].type == CANCER && matrix[i][j].health > strongest.health)
            {
                strongest = matrix[i][j];
            }
        }
    }
    return strongest;
}

int main(int argc, char **argv)
{
    int rank, size;

    // MPI logic starts here
    MPI_Init(&argc, &argv);

    // Get process rank and total number of processes (size)
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Random
    srand(time(0) * rank);

    // Create and commit cell structure
    MPI_Datatype mycelltype;

    int count = 1;
    int lengths[1] = { 2 };
    MPI_Aint displacements[1] = { 0 };
    MPI_Datatype types[1] = { MPI_INT };

    MPI_Type_create_struct(count, lengths, displacements, types, &mycelltype);
    MPI_Type_commit(&mycelltype);

    // Register reduction operation
    MPI_Op MPI_MY_MAX;
    MPI_Op_create( (MPI_User_function *) myMax, 1, &MPI_MY_MAX);

    // Data prepare
    cell **matrix;
    matrix = allocateMatrix(M, N);

    cell *bottom_row = allocateArray(N);

    if (rank == 0)
    {
        // Allocate matrix
        // Set matrix
        matrix = setMatrix(matrix, M, N);
    }

    // Sync matrix
    for (int i = 0; i < M; i++)
    {
        MPI_Bcast(matrix[i], N, mycelltype, 0, MPI_COMM_WORLD);
    }

    // Define submatrix for each process
    int startM = rank * (M / size);
    int endM = startM + (M / size) - 1;

    // Simulate
    int step = 0;

    // Loop through steps
    for (int i = 0; i < STEPS; i++)
    {
        // Simulate
        doWork(matrix, startM, endM, N);

        // Sync corner rows
        // upper cells
        if (rank != 0)
        {
            MPI_Send(matrix[startM], N, mycelltype, rank - 1, 40, MPI_COMM_WORLD);
            MPI_Recv(bottom_row, N, mycelltype, rank - 1, 40, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            doWorkEdge(matrix, bottom_row, N, startM);
        }
        // lower cells
        if (rank != size - 1)
        {
            MPI_Send(matrix[endM], N, mycelltype, rank + 1, 40, MPI_COMM_WORLD);
            MPI_Recv(bottom_row, N, mycelltype, rank + 1, 40, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
            doWorkEdge(matrix, bottom_row, N, startM);
        }
    }

    // Find strongest
    // Find local strongest
    cell strongest = findLocalStrongestCancerCell(matrix, startM, endM, N);
    printf("[%d]: Strongest local: %d_%d\n", rank, strongest.type, strongest.health);

    // Find global strongest with custom redustion
    int strongestGlobal; 
    MPI_Reduce(&strongest.health, &strongestGlobal, 1, MPI_INT, MPI_MY_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) printf("Strongest global: %d\n", strongestGlobal);

    // Cleanup and finish
    MPI_Op_free(&MPI_MY_MAX);
    MPI_Finalize();

    return 0;
}