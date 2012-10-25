/**
 * @file
 * The Master node (MPI Blocking communication)
 */
#include "Taskfarm.h"

/**
 * @brief Performs master node operations
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int MasterBlocking(module *m, pool *p) {
  int mstat = SUCCESS;

  storage *send_buffer, *recv_buffer;
  int i = 0, k = 0, cid = 0, terminated_nodes = 0;
  int tag;
  int header[HEADER_SIZE];
  int c_offset = 0;

  MPI_Status mpi_status;
  int completed = 0, send_node;

  task *t = NULL;
  checkpoint *c = NULL;

  /* Data buffers */
  send_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  /* Initialize data buffers */
  send_buffer->layout.rank = c->storage->layout.rank;
  send_buffer->layout.dim[0] = 1;
  send_buffer->layout.dim[1] = c->storage->layout.dim[1];
  send_buffer->data = AllocateBuffer(send_buffer->layout.rank, send_buffer->layout.dim);
  if (!send_buffer->data) Error(CORE_ERR_MEM);

  send_buffer->layout.size = sizeof(int) * (HEADER_SIZE);
  for (k = 0; k < m->task_banks; k++) {
    send_buffer->layout.size +=
      GetSize(p->task->storage[k].layout.rank, p->task->storage[k].layout.dim)*p->task->storage[k].layout.datatype_size;
  }
  mstat = Allocate(send_buffer, send_buffer->layout.size, sizeof(char));

  recv_buffer->layout.rank = c->storage->layout.rank;
  recv_buffer->layout.dim[0] = 1;
  recv_buffer->layout.dim[1] = c->storage->layout.dim[1];
  recv_buffer->data = AllocateBuffer(recv_buffer->layout.rank, recv_buffer->layout.dim);
  if (!recv_buffer->data) Error(CORE_ERR_MEM);

  recv_buffer->layout.size = send_buffer->layout.size;
  mstat = Allocate(recv_buffer, recv_buffer->layout.size, sizeof(char));

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = TaskPrepare(m, p, t);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, send_buffer->memory, p, t, TAG_DATA);
      CheckStatus(mstat);
    } else {
      tag = TAG_TERMINATE;
      memcpy(&send_buffer->memory, &tag, sizeof(int));
      terminated_nodes++;
    }

    MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
        i, TAG_DATA, MPI_COMM_WORLD);
  }

  /* The task farm loop (Blocking communication) */
  while (1) {

    /* Flush checkpoint buffer and write data, reset counter */
    if (c->counter > (c->size-1)) {
      mstat = CheckpointPrepare(m, p, c);
      CheckStatus(mstat);

      mstat = CheckpointProcess(m, p, c);
      CheckStatus(mstat);

      cid++;

      /* Reset the checkpoint */
      CheckpointReset(m, p, c, cid);
    }

    /* Wait for any operation to complete */
    MPI_Recv(&(recv_buffer->memory[0]), recv_buffer->layout.size, MPI_CHAR,
      MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

      send_node = mpi_status.MPI_SOURCE;

    /* Get the data header */
    memcpy(header, recv_buffer->memory, HEADER_SIZE*sizeof(int));

    c_offset = c->counter*(int)recv_buffer->layout.size;
    memcpy(c->storage->memory + c_offset, recv_buffer->memory, recv_buffer->layout.size);

    p->board->data[header[3]][header[4]] = (double)header[2];

    c->counter++;
    completed++;

    mstat = TaskPrepare(m, p, t);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {

      mstat = Pack(m, send_buffer->memory, p, t, TAG_DATA);
      CheckStatus(mstat);

      MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
          send_node, TAG_DATA, MPI_COMM_WORLD);

    }

    if (completed == p->pool_size) break;
  }

  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    send_buffer->data[0][0] = (double) TAG_TERMINATE;
    tag = TAG_TERMINATE;
    memcpy(send_buffer->memory, &tag, sizeof(int));

    MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
        i, TAG_DATA, MPI_COMM_WORLD);

  }

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (send_buffer) {
    if (send_buffer->data) FreeBuffer(send_buffer->data);
    if (send_buffer->memory) Free(send_buffer);
    free(send_buffer);
  }

  if (recv_buffer) {
    if (recv_buffer->data) FreeBuffer(recv_buffer->data);
    if (recv_buffer->memory) Free(recv_buffer);
    free(recv_buffer);
  }

  return mstat;
}

