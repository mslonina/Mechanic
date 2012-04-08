/**
 * @file
 * The Master node
 */
#include "Taskfarm.h"

/**
 * @function
 * Performs master node operations
 */
int Master(module *m, pool *p) {
  int mstat = 0;

  storage *send_buffer, *recv_buffer;
  int i = 0, k = 0, cid = 0, terminated_nodes = 0;

  MPI_Status *send_status, *recv_status, mpi_status;
  MPI_Request *send_request, *recv_request;
  int *intags, index = 0, req_flag, send_node;

  task *t = NULL;
  checkpoint *c = NULL;

  /* Message buffers */
  intags = calloc(m->mpi_size * sizeof(uintptr_t), sizeof(uintptr_t));
  if (!intags) Error(CORE_ERR_MEM);

  send_status = calloc((m->mpi_size-1) * sizeof(MPI_Status), sizeof(MPI_Status));
  if (!send_status) Error(CORE_ERR_MEM);

  send_request = calloc((m->mpi_size-1) * sizeof(MPI_Request), sizeof(MPI_Request));
  if (!send_request) Error(CORE_ERR_MEM);

  recv_status = calloc((m->mpi_size-1) * sizeof(MPI_Status), sizeof(MPI_Status));
  if (!recv_status) Error(CORE_ERR_MEM);

  recv_request = calloc((m->mpi_size-1) * sizeof(MPI_Request), sizeof(MPI_Request));
  if (!recv_request) Error(CORE_ERR_MEM);

  /* Data buffers */
  send_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize message tags */
  for (i = 0; i < m->mpi_size; i++) intags[i] = i;

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  /* Initialize data buffers */
  send_buffer->layout.rank = c->storage->layout.rank;
  send_buffer->layout.dim[0] = c->storage->layout.dim[0];
  send_buffer->layout.dim[1] = c->storage->layout.dim[1];
  send_buffer->data = AllocateBuffer(send_buffer->layout.rank, send_buffer->layout.dim);
  if (!send_buffer->data) Error(CORE_ERR_MEM);

  recv_buffer->layout.rank = c->storage->layout.rank;
  recv_buffer->layout.dim[0] = c->storage->layout.dim[0];
  recv_buffer->layout.dim[1] = c->storage->layout.dim[1];
  recv_buffer->data = AllocateBuffer(recv_buffer->layout.rank, recv_buffer->layout.dim);
  if (!recv_buffer->data) Error(CORE_ERR_MEM);

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = TaskPrepare(m, p, t);

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, &send_buffer->data[i-1][0], send_buffer->layout.dim[1], p, t, TAG_DATA);
      CheckStatus(mstat);
    } else {
      send_buffer->data[i-1][0] = (double) TAG_TERMINATE;
      terminated_nodes++;
    }

    MPI_Isend(&send_buffer->data[i-1][0], send_buffer->layout.dim[1], MPI_DOUBLE,
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&recv_buffer->data[i-1][0], recv_buffer->layout.dim[1], MPI_DOUBLE,
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);
  }
  MPI_Waitall(m->mpi_size - 1, send_request, send_status);

  /* The task farm loop (Non-blocking communication) */
  c->counter = 0;
  while (1) {

    /* Test for any completed request */
    MPI_Testany(m->mpi_size-1, recv_request, &index, &req_flag, &mpi_status);

    if (!req_flag) {

      /* Flush checkpoint buffer and write data, reset counter */
      if (c->counter > (c->size-1)) {
        mstat = CheckpointPrepare(m, p, c);
        CheckStatus(mstat);

        mstat = CheckpointProcess(m, p, c);
        CheckStatus(mstat);

        cid++;

        /* Reset the checkpoint */
        CheckpointReset(m, p, c, cid);
        c->counter = 0;
      }

      /* Wait for any operation to complete */
      MPI_Waitany(m->mpi_size-1, recv_request, &index, &mpi_status);
      send_node = index+1;

      /* Copy data to the checkpoint buffer */
      for (k = 0; k < recv_buffer->layout.dim[1]; k++) {
        c->storage->data[c->counter][k] = recv_buffer->data[index][k];
      }
      p->board->data[(int)c->storage->data[c->counter][3]][(int)c->storage->data[c->counter][4]] = c->storage->data[c->counter][2];

      c->counter++;

      mstat = TaskPrepare(m, p, t);
      if (mstat != NO_MORE_TASKS) {

        mstat = Pack(m, &send_buffer->data[index][0], send_buffer->layout.dim[1], p, t, TAG_DATA);
        CheckStatus(mstat);

        MPI_Isend(&send_buffer->data[index][0], send_buffer->layout.dim[1], MPI_DOUBLE,
            send_node, intags[send_node], MPI_COMM_WORLD, &send_request[index]);
        MPI_Irecv(&recv_buffer->data[index][0], recv_buffer->layout.dim[1], MPI_DOUBLE,
            send_node, intags[send_node], MPI_COMM_WORLD, &recv_request[index]);

        MPI_Wait(&send_request[index], &send_status[index]);
      }
    }
    if (index == MPI_UNDEFINED) break;
  }

  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  CheckpointReset(m, p, c, cid);
  c->counter = 0;

  /* Receive outstanding data */
  MPI_Waitall(m->mpi_size - 1, send_request, send_status);
  MPI_Waitall(m->mpi_size - 1, recv_request, recv_status);

/*  for (i = 0; i < recv_buffer->layout.dim[0]; i++) {
    for (k = 0; k < recv_buffer->layout.dim[1]; k++) {
      c->storage->data[i][k] = recv_buffer->data[i][k];
    }
    p->board->data[(int)c->storage->data[i][3]][(int)c->storage->data[i][4]] = c->storage->data[i][2];
  }
*/
  /* Process the last checkpoint */
//  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

//  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    send_buffer->data[i-1][0] = (double) TAG_TERMINATE;

    MPI_Isend(&send_buffer->data[i-1][0], send_buffer->layout.dim[1], MPI_DOUBLE,
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&recv_buffer->data[i-1][0], recv_buffer->layout.dim[1], MPI_DOUBLE,
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);

  }

  MPI_Waitall(m->mpi_size - 1, send_request, send_status);
  MPI_Waitall(m->mpi_size - 1, recv_request, recv_status);

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (intags) free(intags);
  if (send_status) free(send_status);
  if (send_request) free(send_request);
  if (recv_status) free(recv_status);
  if (recv_request) free(recv_request);

  if (send_buffer) {
    if (send_buffer->data) FreeBuffer(send_buffer->data, send_buffer->layout.dim);
    free(send_buffer);
  }

  if (recv_buffer) {
    if (recv_buffer->data) FreeBuffer(recv_buffer->data, recv_buffer->layout.dim);
    free(recv_buffer);
  }

  return mstat;
}
