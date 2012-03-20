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

  MPI_Status mpi_status;
  MPI_Request mpi_request;

  task *t = NULL;

  node = m->node;
  intag = node;
  
  buffer_rank = 2;
  
  buffer_dims[0] = 1;
  buffer_dims[1] = m->mpi_size + 2 + MAX_RANK; // offset: tag, tid, location
  for (i = 0; i < m->task_banks; i++) {
    buffer_dims[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }
    
  send_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);
  recv_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);

  t = TaskLoad(m, p, 0);
//goto finalize;

  while (1) {

    MPI_Irecv(&recv_buffer[0][0], buffer_dims[1], MPI_DOUBLE, 
        MASTER, intag, MPI_COMM_WORLD, &mpi_request);
    MPI_Wait(&mpi_request, &mpi_status);

    Unpack(m, &recv_buffer[0][0], buffer_dims[1], p, t, &tag);

    if (tag != TAG_TERMINATE) {

      mstat = TaskPrepare(m, p, t);
      mstat = TaskProcess(m, p, t);

    }

//    printf("Worker %d received tid = %d and tag = %d at location %d, %d\n", 
//        node, t.tid, tag, t.location[0], t.location[1]);

    Pack(m, &send_buffer[0][0], buffer_dims[1], p, t, tag);

    MPI_Isend(&send_buffer[0][0], buffer_dims[1], MPI_DOUBLE, 
        MASTER, intag, MPI_COMM_WORLD, &mpi_request);

    if (tag == TAG_TERMINATE) break;
  }
finalize:
  TaskFinalize(m, p, t);

  /* Free the buffer */
  if (send_buffer) FreeDoubleArray(send_buffer, buffer_dims);
  if (recv_buffer) FreeDoubleArray(recv_buffer, buffer_dims);

  return mstat;
}
