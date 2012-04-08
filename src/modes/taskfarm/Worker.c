/**
 * @file
 * The Worker node
 */
#include "Taskfarm.h"

/**
 * @function
 * Performs worker node operations
 */
int Worker(module *m, pool *p) {
  int mstat = 0;
  int node, tag, intag;

  MPI_Status recv_status, send_status;
  MPI_Request send_request, recv_request;

  task *t = NULL;
  checkpoint *c = NULL;
  storage *send_buffer = NULL, *recv_buffer = NULL;

  node = m->node;
  intag = node;

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  /* Data buffers */
  send_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize data buffers */
  send_buffer->layout.rank = 2;
  send_buffer->layout.dim[0] = 1;
  send_buffer->layout.dim[1] = c->storage->layout.dim[1];
  send_buffer->data = AllocateBuffer(send_buffer->layout.rank, send_buffer->layout.dim);
  if (!send_buffer->data) Error(CORE_ERR_MEM);

  recv_buffer->layout.rank = 2;
  recv_buffer->layout.dim[0] = 1;
  recv_buffer->layout.dim[1] = c->storage->layout.dim[1];
  recv_buffer->data = AllocateBuffer(recv_buffer->layout.rank, recv_buffer->layout.dim);
  if (!recv_buffer->data) Error(CORE_ERR_MEM);

  while (1) {

    MPI_Irecv(&recv_buffer->data[0][0], recv_buffer->layout.dim[1], MPI_DOUBLE,
        MASTER, intag, MPI_COMM_WORLD, &recv_request);
    MPI_Wait(&recv_request, &recv_status);

    mstat = Unpack(m, &recv_buffer->data[0][0], p, t, &tag);
    CheckStatus(mstat);

    if (tag != TAG_TERMINATE) {

      mstat = TaskPrepare(m, p, t);
      CheckStatus(mstat);

      mstat = TaskProcess(m, p, t);
      CheckStatus(mstat);

    }

    t->status = TASK_FINISHED;

    mstat = Pack(m, &send_buffer->data[0][0], p, t, tag);
    CheckStatus(mstat);

    MPI_Isend(&send_buffer->data[0][0], send_buffer->layout.dim[1], MPI_DOUBLE,
        MASTER, intag, MPI_COMM_WORLD, &send_request);
    MPI_Wait(&send_request, &send_status);

    if (tag == TAG_TERMINATE) break;
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
