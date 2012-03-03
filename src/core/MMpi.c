/**
 * @file
 */
#include "MMpi.h"

/**
 * @function
 * Builds MPI derived type for LRC_configDefaults
 */
int LRC_datatype(LRC_configDefaults c, MPI_Datatype *mpi_t) {
  int mstat = 0, i = 0;
  int block_lengths[4];
  MPI_Aint displacements[4];
  MPI_Datatype types[4];
  MPI_Aint addresses[5];

  block_lengths[0] = LRC_CONFIG_LEN;
  block_lengths[1] = LRC_CONFIG_LEN;
  block_lengths[2] = LRC_CONFIG_LEN;
  types[0] = MPI_CHAR;
  types[1] = MPI_CHAR;
  types[2] = MPI_CHAR;
  types[3] = MPI_INT;

  MPI_Get_address(&c, &addresses[0]);
  MPI_Get_address(&c.space, &addresses[1]);
  MPI_Get_address(&c.name, &addresses[2]);
  MPI_Get_address(&c.value, &addresses[3]);
  MPI_Get_address(&c.type, &addresses[4]);

  for (i = 0; i < 4; i++) {
    displacements[i] = addresses[i+1] - addresses[0];
  }

  mstat = MPI_Type_struct(4, block_lengths, displacements, types, mpi_t);
  mstat = MPI_Type_commit(mpi_t);

  return mstat;
}
