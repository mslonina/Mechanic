/**
 * @file
 * The MPI-related functions and wrappers
 */
#include "MMpi.h"

/**
 * @brief Build MPI derived type for LRC_configDefaults
 *
 * @param c The input LRC defaults structure
 * @param mpi_t The output MPI derived type
 *
 * @return 0 on success, error code otherwise
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
  CheckStatus(mstat);

  mstat = MPI_Type_commit(mpi_t);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief Pack the task data into 1D contigous array
 *
 * @param m The module pointer
 * @param buffer The output pack buffer
 * @param p The current pool pointer
 * @param t The input task pointer
 * @param tag The MPI message tag
 *
 * @return 0 on success, error code otherwise
 */
int Pack(module *m, double *buffer, pool *p, task *t, int tag) {
  int mstat = 0;
  int position = 0;
  int i = 0;
  int size = 0;

  buffer[0] = (double) tag; /* Message tag */
  position++;

  if (tag != TAG_TERMINATE) {
    buffer[1] = (double) t->tid; position++; /* Task ID */
    buffer[2] = (double) t->status; position++; /* Task status */

    /* Task location in the pool */
    for (i = 0; i < p->board->layout.rank; i++) {
      buffer[i+position] = t->location[i];
    }
    position = position + p->board->layout.rank;

    /* Task data */
    for (i = 0; i < m->task_banks; i++) {
      Array2Vec(buffer+position, t->storage[i].data,
          t->storage[i].layout.rank, t->storage[i].layout.dim);
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dim);
      position = position + size;
    }

    /* Mark the task on board */
    if (m->node == MASTER) {
      p->board->data[t->location[0]][t->location[1]] = TASK_IN_USE;
    }
  }

  return mstat;
}

/**
 * @brief Unpack the 1D-contigous array into task structure
 *
 * @param m The module pointer
 * @param buffer The input pack buffer
 * @param p The current pool pointer
 * @param t The output task pointer
 * @param tag The MPI message tag
 *
 * @return 0 on success, error code otherwise
 */
int Unpack(module *m, double *buffer, pool *p, task *t, int *tag) {
  int mstat = 0;
  int position = 0;
  int i = 0;
  int size = 0;

  *tag = (int)buffer[0]; /* Message tag */
  position++;

  if (*tag != TAG_TERMINATE) {
    t->tid = (int)buffer[1]; position++; /* Task ID*/
    t->status = (int)buffer[2]; position++; /* Task status */

    /* Task location in the pool */
    for (i = 0; i < p->board->layout.rank; i++) {
      t->location[i] = buffer[i+position];
    }
    position = position + p->board->layout.rank;

    /* Task data */
    for (i = 0; i < m->task_banks; i++) {
      Vec2Array(buffer+position, t->storage[i].data,
          t->storage[i].layout.rank, t->storage[i].layout.dim);
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dim);
      position = position + size;
    }

    /* Mark the task on board */
    if (m->node == MASTER) {
      p->board->data[t->location[0]][t->location[1]] = t->status;
    }
  }

  return mstat;
}

