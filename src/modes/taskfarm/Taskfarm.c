/**
 * @file
 * The Task Farm
 */
#include "Taskfarm.h"

/**
 * @brief The MPI Task Farm main function
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
 */
int Taskfarm(module *m) {
  int mstat = SUCCESS;
  int pid = 0;
  pool **p = NULL;
  int pool_create;

  double cpu_time;
  clock_t time_in, time_out;

  /* Prepare the simulation */
  mstat = Prepare(m);
  CheckStatus(mstat);

  /**
   * (A) Initialize the pool bank
   */
  p = calloc(m->layer.init.pools * sizeof(pool*), sizeof(pool*));
  if (!p) Error(CORE_ERR_MEM);

  for (pid = 0; pid < m->layer.init.pools; pid++) {
    p[pid] = PoolLoad(m, pid);
  }

  pid = 0; // reset pid
  pool_create = POOL_CREATE_NEW;

  /**
   * (B) The Restart mode
   * We try to recreate the memory layout for each pool and read stored data
   */
  if (m->mode == RESTART_MODE) {
    if (m->node == MASTER) {
      Message(MESSAGE_INFO, "Recreating the pool loop\n");
    }
    mstat = Restart(m, p, &pid);
    CheckStatus(mstat);
  }
  
  /**
   * (C) The Pool loop
   */
  do {

    /* Pool storage */
    if (m->mode != RESTART_MODE) Storage(m, p[pid]);

    do {
      if (m->node == MASTER) {
        Message(MESSAGE_INFO, "Entering the pool %04d\n", p[pid]->pid);
      }
      
      time_in = clock();
      
      mstat = NodePrepare(m, p, p[pid]);
      CheckStatus(mstat);
      
      if (m->mode != RESTART_MODE) {
        mstat = PoolReset(m, p[pid]);
        CheckStatus(mstat);
      }

      mstat = PoolPrepare(m, p, p[pid]);
      CheckStatus(mstat);

      /**
       * The Task loop
       */
      if (m->node == MASTER) {
//        if (m->communication_type == MPI_BLOCKING) {
          mstat = MasterBlocking(m, p[pid]);
//        } else {
//          mstat = Master(m, p[pid]);
//        }
        CheckStatus(mstat);
      } else {
//        if (m->communication_type == MPI_BLOCKING) {
          mstat = WorkerBlocking(m, p[pid]);
//        } else {
//          mstat = Worker(m, p[pid]);
//        }
        CheckStatus(mstat);
      }

      pool_create = PoolProcess(m, p, p[pid]);
      
      mstat = NodeProcess(m, p, p[pid]);
      CheckStatus(mstat);
      
      p[pid]->rid++;
      time_out = clock();
      cpu_time = (double)(time_out - time_in)/CLOCKS_PER_SEC;
      if (m->node == MASTER) {
        Message(MESSAGE_INFO, "Pool %04d computed. CPU time: %f\n", p[pid]->pid, cpu_time);
      }
    } while (pool_create == POOL_RESET);

    pid++;

    /* Security check */
    if (pid == m->layer.init.pools) pool_create = POOL_FINALIZE;

    /* Revert to normal mode, after the restarted pool is finished */
    if (m->mode == RESTART_MODE && pool_create == POOL_CREATE_NEW) m->mode = NORMAL_MODE;
  } while (pool_create != POOL_FINALIZE);

  /* Process the simulation */
  mstat = Process(m, p);
  CheckStatus(mstat);

  /* Finalize the pool bank */
  for (pid = 0; pid < m->layer.init.pools; pid++) {
    PoolFinalize(m, p[pid]);
  }
  free(p);

  return mstat;
}
