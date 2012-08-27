/**
 * @file
 * The Worker node (MPI Blocking communication)
 */
#include "Taskfarm.h"

/**
 * @brief Performs worker node operations
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int WorkerBlocking(module *m, pool *p) {
  int mstat = SUCCESS;
  int node, tag;

  MPI_Status recv_status;

  task *t = NULL;
  checkpoint *c = NULL;
  storage *send_buffer = NULL, *recv_buffer = NULL;

  node = m->node;

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

    MPI_Recv(&recv_buffer->data[0][0], recv_buffer->layout.dim[1], MPI_DOUBLE,
        MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_status);

    mstat = Unpack(m, &recv_buffer->data[0][0], p, t, &tag);
    CheckStatus(mstat);

    if (tag == TAG_TERMINATE) {
      break;
    } else {

      mstat = TaskPrepare(m, p, t);
      CheckStatus(mstat);

      mstat = TaskProcess(m, p, t);
      CheckStatus(mstat);

      t->status = TASK_FINISHED;

      mstat = Pack(m, &send_buffer->data[0][0], p, t, tag);
      CheckStatus(mstat);

      MPI_Send(&send_buffer->data[0][0], send_buffer->layout.dim[1], MPI_DOUBLE,
          MASTER, TAG_DATA, MPI_COMM_WORLD);

    }
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
