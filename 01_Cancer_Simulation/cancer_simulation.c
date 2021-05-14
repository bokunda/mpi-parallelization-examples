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

#define M 10
#define N 10
#define STEPS 10

typedef struct Cell {
    int type;
    int strength;
} cell;

// Manually implemented MPI_MAX function
void myMax(int *invector, int *outvector, int *len, MPI_Datatype type)
{
    *outvector = *invector > *outvector ? *invector : *outvector;
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
    MPI_Aint extentINT, lb;

    int count = 2;
    int lengths[2] = { 1, 1 };
    MPI_Aint displacements[2] = { 0, extentINT };
    MPI_Datatype types[2] = { MPI_INT, MPI_INT };

    MPI_Type_get_extent(MPI_INT, &lb, &extentINT);

    MPI_Type_create_struct(count, lengths, displacements, types, &mycelltype);
    MPI_Type_commit(&mycelltype);

    // Register reduction operation
    MPI_Op MPI_MY_MAX;
    MPI_Op_create( (MPI_User_function *) myMax, 1, &MPI_MY_MAX);

    // Data prepare
    // TODO: Allocate matrix
    // TODO: Set matrix
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