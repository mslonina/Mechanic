/**
 * @file
 * The Worker node 
 */
#include "Taskfarm.h"

/**
 * @function
 * Performes worker node operations
 */
int Worker(module *m, pool *p) {
  int mstat = 0;
  double **send_buffer = NULL, **recv_buffer = NULL;
  int buffer_dims[2], buffer_rank;
  int i, node, tag, intag;

  MPI_Status recv_status;
  MPI_Request send_request, recv_request;

  task *t = NULL;

  node = m->node;
  intag = node;
  
  buffer_rank = 2;
  
  buffer_dims[0] = 1;
  buffer_dims[1] = m->mpi_size + 2 + MAX_RANK; // offset: tag, tid, location
  for (i = 0; i < m->task_banks; i++) {
    buffer_dims[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }
    
  send_buffer = AllocateBuffer(buffer_rank, buffer_dims);
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = AllocateBuffer(buffer_rank, buffer_dims);
  if (!recv_buffer) Error(CORE_ERR_MEM);

  t = TaskLoad(m, p, 0);

  while (1) {

    MPI_Irecv(&recv_buffer[0][0], buffer_dims[1], MPI_DOUBLE, 
        MASTER, intag, MPI_COMM_WORLD, &recv_request);
    MPI_Wait(&recv_request, &recv_status);

    mstat = Unpack(m, &recv_buffer[0][0], buffer_dims[1], p, t, &tag);
    CheckStatus(mstat);

    if (tag != TAG_TERMINATE) {

      mstat = TaskPrepare(m, p, t);
      CheckStatus(mstat);

      mstat = TaskProcess(m, p, t);
      CheckStatus(mstat);

    }

    mstat = Pack(m, &send_buffer[0][0], buffer_dims[1], p, t, tag);
    CheckStatus(mstat);

    MPI_Isend(&send_buffer[0][0], buffer_dims[1], MPI_DOUBLE, 
        MASTER, intag, MPI_COMM_WORLD, &send_request);

    if (tag == TAG_TERMINATE) break;
  }
  
  /* Finalize */
  TaskFinalize(m, p, t);

  if (send_buffer) FreeBuffer(send_buffer, buffer_dims);
  if (recv_buffer) FreeBuffer(recv_buffer, buffer_dims);

  return mstat;
}
