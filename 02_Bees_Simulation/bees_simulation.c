#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define NO_HIVE 0
#define EUROPEAN_HIVE 1
#define AFRICAN_HIVE 2

#define EUROPEAN_HIVE_START_I 1
#define EUROPEAN_HIVE_START_J 1
#define EUROPEAN_HIVE_INITIAL_SIZE 10000
#define EUROPEAN_HIVE_INCREMENT 3000
#define EUROPEAN_HIVE_MAX_SIZE 20000

#define AFRICAN_HIVE_START_I 1
#define AFRICAN_HIVE_START_J 4
#define AFRICAN_HIVE_INITIAL_SIZE 9000
#define AFRICAN_HIVE_INCREMENT 4000
#define AFRICAN_HIVE_MAX_SIZE 26000

#define PRINT_ALL 0
#define PRINT_TYPE 1
#define PRINT_SIZE 2

#define M 10
#define N 10

#define STEPS 100

typedef struct BeeHive {
    int type;
    int size;
} beehive;

beehive* allocateArray(int n)
{
    return (beehive *)malloc(n * sizeof(beehive));
}

beehive** allocateMatrix(int m, int n)
{
    beehive** matrix = (beehive **)malloc(m * sizeof(beehive*));
    for (int i = 0; i < m; i++) matrix[i] = allocateArray(n);
    return matrix;
}

void myMax(beehive *inbuf, beehive *outbuf, int *len, MPI_Datatype type)
{
    *outbuf = (*inbuf).size > (*outbuf).size ? *inbuf : *outbuf;
}

beehive** setMatrix(beehive** matrix, int m, int n)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matrix[i][j].size = 0;
            matrix[i][j].type = NO_HIVE;
        }
    }
    return matrix;
}

void printMatrix(beehive** matrix, int m, int n, int printOption)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0 ; j < n; j++)
        {
            if (printOption == PRINT_ALL)
            {
                printf("[%d]%d ", matrix[i][j].type, matrix[i][j].size);
            }
            else if (printOption == PRINT_SIZE)
            {
                printf("%d ", matrix[i][j].size);
            }
            else
            {
                printf("%d ", matrix[i][j].type);
            }
        }
        printf("\n");
    }
}

beehive getStrongestHive(beehive currentHive, beehive newHive)
{
    return currentHive.size > newHive.size ? currentHive : newHive;
}

void checkForOppositeHives(beehive **matrix, int i, int j, int m, int n)
{
    // check top
    if (i > 0 && matrix[i-1][j].type != matrix[i][j].type)
    {
        matrix[i-1][j] = getStrongestHive(matrix[i-1][j], matrix[i][j]);
    }
    // check right
    else if (j < n - 1 && matrix[i][j+1].type != matrix[i][j].type)
    {
        matrix[i][j+1] = getStrongestHive(matrix[i][j+1], matrix[i][j]);
    }
    // check bottom
    else if (i < m - 1 && matrix[i+1][j].type != matrix[i][j].type)
    {
        matrix[i+1][j] = getStrongestHive(matrix[i+1][j], matrix[i][j]);
    }
    // check left
    else if (j > 0 && matrix[i][j-1].type != matrix[i][j].type)
    {
        matrix[i][j-1] = getStrongestHive(matrix[i][j-1], matrix[i][j]);
    }
}

void tryMigrateHive(beehive **matrix, int i, int j, int m, int n)
{
    if ( (matrix[i][j].type == EUROPEAN_HIVE && matrix[i][j].size > EUROPEAN_HIVE_MAX_SIZE)
    ||
        (matrix[i][j].type == AFRICAN_HIVE && matrix[i][j].size > AFRICAN_HIVE_MAX_SIZE))
    {
        matrix[i][j].size = matrix[i][j].size / 2;
        beehive newHive = matrix[i][j];

        // check top
        if (i > 0 && matrix[i-1][j].type == NO_HIVE)
        {
            matrix[i-1][j] = newHive;
        }
        // check right 
        else if (j < n - 1 && matrix[i][j+1].type == NO_HIVE)
        {
            matrix[i][j+1] = newHive;
        }
        // check bottom
        else if (i < m - 1 && matrix[i+1][j].type == NO_HIVE)
        {
            matrix[i+1][j] = newHive;
        }
        // check left
        else if (j > 0 && matrix[i][j-1].type == NO_HIVE)
        {
            matrix[i][j-1] = newHive;
        }
        // if there are no empty fields for a new hive, check if there are opposite hives 
        else
        {
            checkForOppositeHives(matrix, i, j, m, n);
        }
    }
}

void increaseHive(beehive **matrix, int i, int j, int m, int n)
{
    // Increase hive
    if (matrix[i][j].type == EUROPEAN_HIVE)
    {
        matrix[i][j].size += EUROPEAN_HIVE_INCREMENT;
                
        // Check should migrate hive
        tryMigrateHive(matrix, i, j, m, n);
    }
    else if (matrix[i][j].type == AFRICAN_HIVE)
    {
        matrix[i][j].size += AFRICAN_HIVE_INCREMENT;           

        // Check should migrate hive
        tryMigrateHive(matrix, i, j, m, n);
    }
}

void doWork(beehive **matrix, int startM, int endM, int n)
{
    for (int i = startM; i < endM; i++)
    {
        for (int j = 0; j < n; j++)
        {
            increaseHive(matrix, i, j, endM, n);
        }
    }
}

int flag = 0;
void handleEdges(beehive **matrix, beehive *array, int m, int n)
{
    for (int i = 0; i < n; i++)
    {
        matrix[m][i] = getStrongestHive(matrix[m][i], array[i]);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Define new type
    MPI_Datatype beehivetype;
    
    int count = 1;
    int lengths[1] = { 2 };
    MPI_Aint displacements[1] = { 0 };
    MPI_Datatype types[1] = { MPI_INT };

    MPI_Type_create_struct(count, lengths, displacements, types, &beehivetype);
    MPI_Type_commit(&beehivetype);

    // Define reduction operation
    MPI_Op MPI_MY_MAX;
    MPI_Op_create((MPI_User_function *)myMax, 1, &MPI_MY_MAX);

    // Allocate matrix
    beehive** matrix = allocateMatrix(M, N);
    beehive* edgeArray = allocateArray(N);

    // Set matrix - set matrix to NO_HIVE
    matrix = setMatrix(matrix, M, N);

    // Set European hive start position and size
    matrix[EUROPEAN_HIVE_START_I][EUROPEAN_HIVE_START_J].type = EUROPEAN_HIVE;
    matrix[EUROPEAN_HIVE_START_I][EUROPEAN_HIVE_START_J].size = EUROPEAN_HIVE_INITIAL_SIZE;

    // Set African hive start position and size
    matrix[AFRICAN_HIVE_START_I][AFRICAN_HIVE_START_J].type = AFRICAN_HIVE;
    matrix[AFRICAN_HIVE_START_I][AFRICAN_HIVE_START_J].size = AFRICAN_HIVE_INITIAL_SIZE;

    // Define startM & stopM for submatrix
    int startM = rank * (M / size);
    int endM = rank == size - 1 ? M : startM + (M / size);

    // Simulate
    for (int i = 0; i < STEPS; i++)
    {
        doWork(matrix, startM, endM, N);

        // handle top
        if (rank != 0)
        {
            MPI_Send(matrix[startM], N, beehivetype, rank - 1, 10, MPI_COMM_WORLD);
            MPI_Recv(edgeArray, N, beehivetype, rank - 1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            handleEdges(matrix, edgeArray, startM, N);
        }
            
        // handle bottom
        if (rank != size -1)
        {
            MPI_Send(matrix[endM-1], N, beehivetype, rank + 1, 10, MPI_COMM_WORLD);
            MPI_Recv(edgeArray, N, beehivetype, rank + 1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            handleEdges(matrix, edgeArray, endM - 1, N);
        }
    }

    // Sync matrix data
    for (int i = 1; i < size; i++)
    {
        for (int j = startM; j < endM; j++)
        {
            MPI_Bcast(matrix[j], N, beehivetype, rank, MPI_COMM_WORLD);
        }
    }

    // Print final matrix (NOTE: sync matrix is not working)
    if (rank == 0) printMatrix(matrix, M, N, PRINT_ALL);

    MPI_Finalize();

    return 0;
}