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
  int mstat = SUCCESS, i = 0;
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
  if (mstat != MPI_SUCCESS) Error(CORE_ERR_MPI);

  mstat = MPI_Type_commit(mpi_t);
  if (mstat != MPI_SUCCESS) Error(CORE_ERR_MPI);

  return mstat;
}

/**
 * @brief Pack the task data into memory buffer 
 *
 * @param m The module pointer
 * @param buffer The output pack buffer
 * @param p The current pool pointer
 * @param t The input task pointer
 * @param tag The MPI message tag
 *
 * @return 0 on success, error code otherwise
 */
int Pack(module *m, void *buffer, pool *p, task *t, int tag) {
  int mstat = SUCCESS, i = 0;
  int header[HEADER_SIZE] = HEADER_INIT;
  size_t position = 0 ,size = 0;

  header[0] = tag;
  header[1] = t->tid;
  header[2] = t->status;
  header[3] = t->location[0];
  header[4] = t->location[1];

  position = sizeof(int)*(HEADER_SIZE);
  mstat = CopyData(header, buffer, sizeof(int) * (HEADER_SIZE));
  CheckStatus(mstat);

  if (tag != TAG_TERMINATE) {

    /* Task data */
    for (i = 0; i < m->task_banks; i++) {
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dim)*t->storage[i].layout.datatype_size;
      if (t->storage[i].layout.sync) {
        mstat = CopyData(t->storage[i].memory, buffer + position, size);
        CheckStatus(mstat);
      }
      position = position + size;
    }

  }

  return mstat;
}

/**
 * @brief Unpack the memory buffer into task structure
 *
 * @param m The module pointer
 * @param buffer The input pack buffer
 * @param p The current pool pointer
 * @param t The output task pointer
 * @param tag The MPI message tag
 *
 * @return 0 on success, error code otherwise
 */
int Unpack(module *m, void *buffer, pool *p, task *t, int *tag) {
  int mstat = SUCCESS, i = 0;
  int header[HEADER_SIZE] = HEADER_INIT;
  size_t position = 0, size = 0;

  position = sizeof(int)*(HEADER_SIZE);
  mstat = CopyData(buffer, header, sizeof(int) * (HEADER_SIZE));
  CheckStatus(mstat);

  *tag = header[0];
  t->tid = header[1];
  t->status = header[2];
  t->location[0]= header[3];
  t->location[1] = header[4];

  if (*tag != TAG_TERMINATE) {

    /* Task data */
    for (i = 0; i < m->task_banks; i++) {
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dim)*t->storage[i].layout.datatype_size;
      if (t->storage[i].layout.sync) {
        mstat = CopyData(buffer + position, t->storage[i].memory, size);
        CheckStatus(mstat);
      }
      position = position + size;
    }

  }

  return mstat;
}

