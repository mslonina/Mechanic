/**
 * @file
 * The Worker node
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
int Worker(module *m, pool *p) {
  int mstat = SUCCESS;
  int node, tag, intag;
  int k = 0;

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
  send_buffer = calloc(1, sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(1, sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize data buffers */
  send_buffer->layout.size = sizeof(int) * (HEADER_SIZE);
  for (k = 0; k < m->task_banks; k++) {
    send_buffer->layout.size +=
      GetSize(p->task->storage[k].layout.rank, p->task->storage[k].layout.dim)*p->task->storage[k].layout.datatype_size;
  }

  recv_buffer->layout.size = send_buffer->layout.size;
  
  mstat = Allocate(send_buffer, send_buffer->layout.size, sizeof(char));
  mstat = Allocate(recv_buffer, recv_buffer->layout.size, sizeof(char));

  while (1) {

    MPI_Irecv(&(recv_buffer->memory), (int)recv_buffer->layout.size, MPI_CHAR,
        MASTER, intag, MPI_COMM_WORLD, &recv_request);
    MPI_Wait(&recv_request, &recv_status);

    mstat = Unpack(m, &recv_buffer->memory, p, t, &tag);
    CheckStatus(mstat);

    if (tag != TAG_TERMINATE) {

      mstat = TaskPrepare(m, p, t);
      CheckStatus(mstat);

      mstat = TaskProcess(m, p, t);
      CheckStatus(mstat);

      t->status = TASK_FINISHED;
      tag = TAG_RESULT;
    }

    mstat = Pack(m, &send_buffer->memory, p, t, tag);
    CheckStatus(mstat);

    MPI_Isend(&(send_buffer->memory), (int)send_buffer->layout.size, MPI_CHAR,
        MASTER, intag, MPI_COMM_WORLD, &send_request);
    MPI_Wait(&send_request, &send_status);

    if (tag == TAG_TERMINATE) break;

  }

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (send_buffer) {
   // if (send_buffer->memory) free(send_buffer->memory); //Free(send_buffer);
    free(send_buffer);
  }

  if (recv_buffer) {
    //if (recv_buffer->memory) Free(recv_buffer);
    //free(&(recv_buffer->memory[0]));
    free(recv_buffer);
  }

  return mstat;
}

