/**
 * @file
 * The Task Farm
 */
#include "Taskfarm.h"

/**
 * @function
 */
int Taskfarm(module *m) {
  int mstat = 0, i = 0;
  int pid = 0, cid = 0, node, comm, tag;
  int *intags;
  int task_counter = 0;
  pool p;
  checkpoint c;
  task t;
  MPI_Status *mpi_status, mpis;
  MPI_Request *mpi_request;
  double **send_buffer;
  double **recv_buffer;
  int buffer_dims[2];
  int buffer_rank;

  int max_tasks;
  int req_flag, index;
  int send_node;
  
  node = m->node;
  comm = m->mpi_size;

  intags = calloc(comm * sizeof(uintptr_t), sizeof(uintptr_t));

  max_tasks = comm*4; // test 

  mpi_status = calloc((comm-1) * sizeof(MPI_Status), sizeof(MPI_Status));
  mpi_request = calloc((comm-1) * sizeof(MPI_Request), sizeof(MPI_Request));

  for (i = 0; i < comm; i++) {
    intags[i] = i;
  }

  if (node == MASTER) {
    m->datafile = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    m->location = H5Gcreate(m->datafile, "Pools", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }

  /**
   * The Pool loop
   */
  pid = 0;
  while (1) {
    p = PoolLoad(m, pid);
    mstat = PoolInit(m, &p);
    CheckStatus(mstat);

    mstat = Storage(m, &p);
    CheckStatus(mstat);

    mstat = PoolPrepare(m, &p);
    CheckStatus(mstat);

    /* Prepare the data buffer */
    buffer_dims[0] = comm - 1;
    buffer_dims[1] = comm + 2 + MAX_RANK; // offset: tag, tid, location
    for (i = 0; i < m->task_banks; i++) {
      buffer_dims[1] += GetSize(p.task.storage[i].layout.rank, p.task.storage[i].layout.dim);
    }
      
    buffer_rank = 2;
    send_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);
    recv_buffer = AllocateDoubleArray(buffer_rank, buffer_dims);

    /**
     * The Task loop
     *
     * @todo
     * - Implement the task storage scheme
     * - Implement the task board
     */
    
    t = TaskLoad(m, &p, 0);

    /* MASTER */
    if (node == MASTER) {
      cid = 0;
      c = CheckpointLoad(m, &p, cid);

      mstat = CheckpointInit(m, &p, &c);
      CheckStatus(mstat);

      /* Send initial tasks to all workers */
      task_counter = 0;
      for (i = 1; i < comm; i++) {
        t.tid = task_counter;
        TaskPrepare(m, &p, &t);
        
        Pack(m, &send_buffer[i-1][0], buffer_dims[1], p, t, TAG_DATA);
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
        MPI_Testany(comm-1, mpi_request, &index, &req_flag, &mpis);

        if (!req_flag) {

          /* Flush checkpoint buffer and write data, reset counter */
          if (c.counter == c.size) {
            mstat = CheckpointPrepare(m, &p, &c);
            CheckStatus(mstat);

            mstat = CheckpointProcess(m, &p, &c);
            CheckStatus(mstat);
            
            CheckpointFinalize(m, &p, &c);
            cid++;
      
            /* Initialize the next checkpoint */
            c = CheckpointLoad(m, &p, cid);

            mstat = CheckpointInit(m, &p, &c);
            CheckStatus(mstat);
            c.counter = 0;
          }
        
          /* Wait for any operation to complete */
          MPI_Waitany(comm-1, mpi_request, &index, &mpis);
          send_node = index+1;

          Unpack(m, &recv_buffer[index][0], buffer_dims[1], p, &c.task[c.counter], &tag);
          c.counter++;
         
          if (task_counter <= max_tasks) {
            t.tid = task_counter;
            TaskPrepare(m, &p, &t);

            Pack(m, &send_buffer[index][0], buffer_dims[1], p, t, TAG_DATA);
        
            printf("Master Buffer tag = %d, tid = %d, location %d, %d to worker %d\n",
                (int)send_buffer[index][0], (int)send_buffer[index][1],
                (int)send_buffer[index][2], (int)send_buffer[index][3],
                send_node
                );
            
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
        mstat = CheckpointPrepare(m, &p, &c);
        CheckStatus(mstat);
        mstat = CheckpointProcess(m, &p, &c);
        CheckStatus(mstat);
      }

      CheckpointFinalize(m, &p, &c);

      /* Terminate all workers */
      for (i = 1; i < comm; i++) {
        send_buffer[i-1][0] = (double) TAG_TERMINATE;

        MPI_Isend(&send_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
            i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
        MPI_Irecv(&recv_buffer[i-1][0], buffer_dims[1], MPI_DOUBLE, 
            i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
        
      }
      
    } else {
      /* WORKER */
      while (1) {

        MPI_Irecv(&recv_buffer[node-1][0], buffer_dims[1], MPI_DOUBLE, 
            MASTER, intags[node], MPI_COMM_WORLD, &mpi_request[node-1]);
        MPI_Wait(&mpi_request[node-1], &mpi_status[node-1]);

        Unpack(m, &recv_buffer[node-1][0], buffer_dims[1], p, &t, &tag);

        if (tag != TAG_TERMINATE) {

          mstat = TaskPrepare(m, &p, &t);
          mstat = TaskProcess(m, &p, &t);

        }

        printf("Worker %d received tid = %d and tag = %d at location %d, %d\n", 
            node, t.tid, tag, t.location[0], t.location[1]);

        Pack(m, &send_buffer[node-1][0], buffer_dims[1], p, t, tag);

        MPI_Isend(&send_buffer[node-1][0], buffer_dims[1], MPI_DOUBLE, 
            MASTER, intags[node], MPI_COMM_WORLD, &mpi_request[node-1]);


        if (tag == TAG_TERMINATE) break;
      }
    } /* The task loop */
      
    TaskFinalize(m, &p, &t);

    mstat = PoolProcess(m, &p); 
    PoolFinalize(m, &p);
  
    /* Free the buffer */
    FreeDoubleArray(send_buffer, buffer_dims);
    FreeDoubleArray(recv_buffer, buffer_dims);

    pid++;
    if (mstat == POOL_FINALIZE) break;
  } /* The Pool loop */

  if (node == MASTER) {
    H5Gclose(m->location);
    H5Fclose(m->datafile);
  }

  free(intags);
  free(mpi_status);
  free(mpi_request);

  return mstat;
}

