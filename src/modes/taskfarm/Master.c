/**
 * @file
 * The Master node 
 */
#include "Taskfarm.h"

/**
 * @function
 * Performes master node operations
 */
int Master(module *m, pool *p) {
  int mstat = 0;

  double **send_buffer = NULL, **recv_buffer = NULL;
  int buffer_dims[2], buffer_rank;
  int i = 0, cid = 0, terminated_nodes = 0;

  MPI_Status *send_status, *recv_status, mpi_status;
  MPI_Request *send_request, *recv_request;
  int *intags, index = 0, req_flag, send_node, tag;

  task *t = NULL;
  checkpoint *c = NULL;
  
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

  for (i = 0; i < m->mpi_size; i++) intags[i] = i;
  
  buffer_rank = 2;
  
  buffer_dims[0] = m->mpi_size - 1;
  buffer_dims[1] = m->mpi_size + 2 + MAX_RANK; // offset: tag, tid, location
  for (i = 0; i < m->task_banks; i++) {
    buffer_dims[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }
    
  send_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);
  if (!recv_buffer) Error(CORE_ERR_MEM);

  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  mstat = CheckpointInit(m, p, c);
  CheckStatus(mstat);
  
  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = TaskPrepare(m, p, t);
    
    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, &send_buffer[i-1][0], buffer_dims[1], p, t, TAG_DATA);
      CheckStatus(mstat);
    } else {
      send_buffer[i-1][0] = (double) TAG_TERMINATE;
      terminated_nodes++;
    }

    MPI_Isend(&send_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&recv_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);
  }
  
  /* The task farm loop (Non-blocking communication) */
  c->counter = 0;
  while (1) {

    /* Test for any completed request */
    MPI_Testany(m->mpi_size-1, recv_request, &index, &req_flag, &mpi_status);

    if (!req_flag) {

      /* Flush checkpoint buffer and write data, reset counter */
      if (c->counter == c->size) {
        mstat = CheckpointPrepare(m, p, c);
        CheckStatus(mstat);

        mstat = CheckpointProcess(m, p, c);
        CheckStatus(mstat);
        
        CheckpointFinalize(m, p, c);
        cid++;
  
        /* Initialize the next checkpoint */
        c = CheckpointLoad(m, p, cid);

        mstat = CheckpointInit(m, p, c);
        CheckStatus(mstat);

        c->counter = 0;
      }
    
      /* Wait for any operation to complete */
      MPI_Waitany(m->mpi_size-1, recv_request, &index, &mpi_status);
      send_node = index+1;

      mstat = Unpack(m, &recv_buffer[index][0], buffer_dims[1], p, c->task[c->counter], &tag);
      CheckStatus(mstat);

      c->counter++;

      mstat = TaskPrepare(m, p, t);
      if (mstat != NO_MORE_TASKS) {

        mstat = Pack(m, &send_buffer[index][0], buffer_dims[1], p, t, TAG_DATA);
        CheckStatus(mstat);
    
        MPI_Isend(&send_buffer[index][0], buffer_dims[1], MPI_DOUBLE, 
            send_node, intags[send_node], MPI_COMM_WORLD, &send_request[index]);
        MPI_Irecv(&recv_buffer[index][0], buffer_dims[1], MPI_DOUBLE, 
            send_node, intags[send_node], MPI_COMM_WORLD, &recv_request[index]);
      }
    } 
    if (index == MPI_UNDEFINED) break;
  }

  /* Process the last checkpoint */
  //if (c->counter > 0 && c->counter < c->size) {
    mstat = CheckpointPrepare(m, p, c);
    CheckStatus(mstat);

    mstat = CheckpointProcess(m, p, c);
    CheckStatus(mstat);
  //}

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    send_buffer[i-1][0] = (double) TAG_TERMINATE;

    MPI_Isend(&send_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&recv_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);
    
  }
  MPI_Waitall(m->mpi_size - 1, recv_request, send_status);

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);
  
  if (intags) free(intags);
  if (send_status) free(send_status);
  if (send_request) free(send_request);
  if (recv_status) free(recv_status);
  if (recv_request) free(recv_request);
  if (send_buffer) FreeDoubleArray(send_buffer, buffer_dims);
  if (recv_buffer) FreeDoubleArray(recv_buffer, buffer_dims);

  return mstat;
}
