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

  recv_buffer->layout.rank = c->storage->layout.rank;
  recv_buffer->layout.dim[0] = 1;
  recv_buffer->layout.dim[1] = c->storage->layout.dim[1];
  recv_buffer->data = AllocateBuffer(recv_buffer->layout.rank, recv_buffer->layout.dim);
  if (!recv_buffer->data) Error(CORE_ERR_MEM);

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = TaskPrepare(m, p, t);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, &send_buffer->data[0][0], p, t, TAG_DATA);
      CheckStatus(mstat);
    } else {
      send_buffer->data[0][0] = (double) TAG_TERMINATE;
      terminated_nodes++;
    }

    MPI_Send(&send_buffer->data[0][0], send_buffer->layout.dim[1], MPI_DOUBLE,
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
    MPI_Recv(&recv_buffer->data[0][0], recv_buffer->layout.dim[1], MPI_DOUBLE,
      MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

      send_node = mpi_status.MPI_SOURCE;

    /* Copy data to the checkpoint buffer */
    for (k = 0; k < recv_buffer->layout.dim[1]; k++) {
      c->storage->data[c->counter][k] = recv_buffer->data[0][k];
    }
    p->board->
      data[(int)c->storage->data[c->counter][3]]
          [(int)c->storage->data[c->counter][4]]
      = c->storage->data[c->counter][2];

    c->counter++;
    completed++;

    mstat = TaskPrepare(m, p, t);
    CheckStatus(mstat);
    if (mstat != NO_MORE_TASKS) {

      mstat = Pack(m, &send_buffer->data[0][0], p, t, TAG_DATA);
      CheckStatus(mstat);

      MPI_Send(&send_buffer->data[0][0], send_buffer->layout.dim[1], MPI_DOUBLE,
          send_node, TAG_DATA, MPI_COMM_WORLD);

    }
    if (completed == p->pool_size) break;
  }

  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  /*cid++;
  CheckpointReset(m, p, c, cid);

  for (i = 0; i < m->mpi_size - 1; i++) {
    for (k = 0; k < recv_buffer->layout.dim[1]; k++) {
      c->storage->data[i][k] = recv_buffer->data[i][k];
    }
    p->board->data[(int)c->storage->data[i][3]][(int)c->storage->data[i][4]] = c->storage->data[i][2];
  }*/

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    send_buffer->data[0][0] = (double) TAG_TERMINATE;

    MPI_Send(&send_buffer->data[0][0], send_buffer->layout.dim[1], MPI_DOUBLE,
        i, TAG_DATA, MPI_COMM_WORLD);

  }

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (send_buffer) {
    if (send_buffer->data) FreeBuffer(send_buffer->data);
    free(send_buffer);
  }

  if (recv_buffer) {
    if (recv_buffer->data) FreeBuffer(recv_buffer->data);
    free(recv_buffer);
  }

  return mstat;
}
