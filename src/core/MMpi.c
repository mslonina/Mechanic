/**
 * @file
 * The MPI-related functions and wrappers.
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

/**
 * @function
 * Packs the task data to 1D contigous array
 */
int Pack(module *m, double *buffer, int buffer_size, pool p, task t, int tag) {
  int mstat = 0;
  int position = 0;
  int i = 0;
  int size = 0;

  buffer[0] = (double) tag; /* Message tag */

  if (tag != TAG_TERMINATE) {
    buffer[1] = (double) t.tid; /* Task ID */

    position = 2;
    
    /* Task location in the pool */
    for (i = 0; i < p.board.rank; i++) {
      buffer[i+position] = t.location[i];
    }
    position = position + p.board.rank;
 
    /* Task data */
    for (i = 0; i < m->task_banks; i++) {
      Array2Vec(buffer+position, t.storage[i].data, t.storage[i].layout.rank, t.storage[i].layout.dim);
      size = GetSize(t.storage[i].layout.rank, t.storage[i].layout.dim);
      position = position + size;
    }
  }

  return mstat;
}

/**
 * @function
 * Unpack the 1D-contigous array to task
 */
int Unpack(module *m, double *buffer, int buffer_size, pool p, task *t, int *tag) {
  int mstat = 0;
  int position = 0;
  int i = 0;
  int size = 0;

  *tag = (int)buffer[0]; /* Message tag */

  if (*tag != TAG_TERMINATE) {
    t->tid = (int)buffer[1]; /* Task ID*/
    
    position = 2;

    /* Task location in the pool */
    for (i = 0; i < p.board.rank; i++) {
      t->location[i] = buffer[i+position];
    }
    position = position + p.board.rank;

    /* Task data */
    for (i = 0; i < m->task_banks; i++) {
      Vec2Array(buffer+position, t->storage[i].data, t->storage[i].layout.rank, t->storage[i].layout.dim);
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dim);
      position = position + size;
    }
  }

  return mstat;
}

