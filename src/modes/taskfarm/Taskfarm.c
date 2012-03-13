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
  int pid = 0, cid = 0, tid = 0, node, comm, tag;
  int *intags;
  int task_counter = 0, tasks = 0;
  pool p;
  checkpoint c;
  query *q;
  MPI_Status *mpi_status, mpis;
  MPI_Request *mpi_request;

  int idata, *odata; // test only
  int max_tasks;
  int req_flag, index;
  int send_node;
  
  node = m->node;
  comm = m->mpi_size;

  intags = calloc(comm * sizeof(uintptr_t), sizeof(uintptr_t));

  max_tasks = comm * 4; // test 

  odata = calloc((comm-1) * sizeof(uintptr_t), sizeof(uintptr_t)); // test data

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

    /**
     * The Task loop
     *
     * @todo
     * - Implement the task storage scheme
     * - Implement the task board
     */

    /* MASTER */
    if (node == MASTER) {
      cid = 0;
      c = CheckpointLoad(m, &p, cid);

      mstat = CheckpointInit(m, &p, &c);
      CheckStatus(mstat);

      /* Send initial tasks to all workers */
      task_counter = 0; 
      for (i = 1; i < comm; i++) {
        idata = task_counter;
        MPI_Isend(&idata, 1, MPI_INT, i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
        MPI_Irecv(&odata[i-1], 1, MPI_INT, i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
        task_counter++;
      }

      /* The task farm loop (Non-blocking communication) */
      while (1) {
        /* Test for any completed request */
        MPI_Testany(comm-1, mpi_request, &index, &req_flag, &mpis);

        if (!req_flag) {

          /* Flush checkpoint buffer and write data, reset counter */
          if (c.counter >= c.size) {
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
          }
        
          /* Wait for any operation to complete */
          MPI_Waitany(comm-1, mpi_request, &index, &mpis);
          send_node = index+1;

          c.data[c.counter] = odata[index];
          c.counter++;
         
          if (task_counter <= max_tasks) {
            idata = task_counter;
            MPI_Isend(&idata, 1, MPI_INT, send_node, intags[send_node], 
                MPI_COMM_WORLD, &mpi_request[index]);
            MPI_Irecv(&odata[index], 1, MPI_INT, send_node, intags[send_node], 
                MPI_COMM_WORLD, &mpi_request[index]);
            task_counter++;
          } 
        } 
        if (index == MPI_UNDEFINED) break;
      }

      if (c.counter > 0 && c.counter < c.size) {
        mstat = CheckpointPrepare(m, &p, &c);
        CheckStatus(mstat);
        mstat = CheckpointProcess(m, &p, &c);
        CheckStatus(mstat);
        CheckpointFinalize(m, &p, &c);
      }

      /* Terminate all workers */
      for (i = 1; i < comm; i++) {
        idata = TAG_TERMINATE;
        MPI_Isend(&idata, 1, MPI_INT, i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
        MPI_Irecv(&odata[i-1], 1, MPI_INT, i, intags[i], MPI_COMM_WORLD, &mpi_request[i-1]);
      }
      
    } else {
      /* WORKER */
      while (1) {
        MPI_Irecv(&idata, 1, MPI_INT, MASTER, intags[node], 
            MPI_COMM_WORLD, &mpi_request[node-1]);
        MPI_Wait(&mpi_request[node-1], &mpi_status[node-1]);
//        printf("worker %d, idata = %d\n", node, idata);
        tag = idata;
        if (idata != TAG_TERMINATE) {
          idata = idata;
        }
        MPI_Isend(&idata, 1, MPI_INT, MASTER, intags[node], 
            MPI_COMM_WORLD, &mpi_request[node-1]);

        if (tag == TAG_TERMINATE) break;
      }
    } /* The task loop */

    mstat = PoolProcess(m, &p); 
    PoolFinalize(m, &p);
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
  free(odata);

  return mstat;
}
