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

  double **send_buffer, **recv_buffer;
  int buffer_dims[2], buffer_rank;
  int i = 0, task_counter = 0, tasks = 0, cid = 0;

  MPI_Status *mpi_status, mpis;
  MPI_Request *mpi_request;
  int *intags, index = 0, req_flag, send_node, tag;

  task t;
  checkpoint c;
  
  intags = calloc(m->mpi_size * sizeof(uintptr_t), sizeof(uintptr_t));
  for (i = 0; i < m->mpi_size; i++) {
    intags[i] = i;
  }

  mpi_status = malloc((m->mpi_size-1) * sizeof(MPI_Status));
  mpi_request = malloc((m->mpi_size-1) * sizeof(MPI_Request));

  tasks = m->mpi_size * 4;

  buffer_rank = 2;
  
  buffer_dims[0] = m->mpi_size - 1;
  buffer_dims[1] = m->mpi_size + 2 + MAX_RANK; // offset: tag, tid, location
  for (i = 0; i < m->task_banks; i++) {
    buffer_dims[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }
    
  send_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);
  recv_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);

  t = TaskLoad(m, p, 0);

  c = CheckpointLoad(m, p, 0);

  mstat = CheckpointInit(m, p, &c);
  CheckStatus(mstat);
  
  /* Send initial tasks to all workers */
  task_counter = 0;
  for (i = 1; i < m->mpi_size; i++) {
    t.tid = task_counter;
    TaskPrepare(m, p, &t);
    
    Pack(m, &send_buffer[i-1][0], buffer_dims[1], p, &t, TAG_DATA);
    MPI_Isend(&send_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
    MPI_Irecv(&recv_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);

    task_counter++;
  }
  
  /* The task farm loop (Non-blocking communication) */
  c.counter = 0;
  while (1) {
    /* Test for any completed request */
    MPI_Testany(m->mpi_size-1, mpi_request, &index, &req_flag, &mpis);

    if (!req_flag) {

      /* Flush checkpoint buffer and write data, reset counter */
      if (c.counter == c.size) {
        mstat = CheckpointPrepare(m, p, &c);
        CheckStatus(mstat);

        mstat = CheckpointProcess(m, p, &c);
        CheckStatus(mstat);
        
        CheckpointFinalize(m, p, &c);
        cid++;
  
        /* Initialize the next checkpoint */
        c = CheckpointLoad(m, p, cid);

        mstat = CheckpointInit(m, p, &c);
        CheckStatus(mstat);
        c.counter = 0;
      }
    
      /* Wait for any operation to complete */
      MPI_Waitany(m->mpi_size-1, mpi_request, &index, &mpis);
      send_node = index+1;

      Unpack(m, &recv_buffer[index][0], buffer_dims[1], p, &c.task[c.counter], &tag);
      c.counter++;

      if (task_counter <= tasks) {
        t.tid = task_counter;
        TaskPrepare(m, p, &t);

        Pack(m, &send_buffer[index][0], buffer_dims[1], p, &t, TAG_DATA);
    
       /* printf("Master Buffer tag = %d, tid = %d, location %d, %d to worker %d\n",
            (int)send_buffer[index][0], (int)send_buffer[index][1],
            (int)send_buffer[index][2], (int)send_buffer[index][3],
            send_node
            );
*/
        MPI_Isend(&send_buffer[index][0], buffer_dims[1], MPI_DOUBLE, 
            send_node, intags[send_node], MPI_COMM_WORLD, &mpi_request[index]);
        MPI_Irecv(&recv_buffer[index][0], buffer_dims[1], MPI_DOUBLE, 
            send_node, intags[send_node], MPI_COMM_WORLD, &mpi_request[index]);
    
        task_counter++;
      } 
    } 
    if (index == MPI_UNDEFINED) break;
  }

  /* Process the last checkpoint */
  if (c.counter > 0 && c.counter < c.size) {
    mstat = CheckpointPrepare(m, p, &c);
    CheckStatus(mstat);
    mstat = CheckpointProcess(m, p, &c);
    CheckStatus(mstat);
  }

  CheckpointFinalize(m, p, &c);

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size; i++) {
    send_buffer[i-1][0] = (double) TAG_TERMINATE;

    MPI_Isend(&send_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
    MPI_Irecv(&recv_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
        i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
    
  }

  TaskFinalize(m, p, &t);

  /* Free the buffer */
  FreeDoubleArray(send_buffer, buffer_dims);
  FreeDoubleArray(recv_buffer, buffer_dims);
  
  free(intags);
  free(mpi_status);
  free(mpi_request);

  return mstat;
}
