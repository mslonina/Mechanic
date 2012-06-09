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
  int mstat = 0;
  int pid = 0;
  pool **p = NULL;
  int pool_create;

  /* Prepare the simulation */
  Prepare(m);

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
  }

  /**
   * (C) The Pool loop
   */
  do {

    /* Pool storage */
    if (m->mode != RESTART_MODE) Storage(m, p[pid]);

    do {
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
        mstat = Master(m, p[pid]);
      } else {
        mstat = Worker(m, p[pid]);
      }

      pool_create = PoolProcess(m, p, p[pid]);
      p[pid]->rid++;
    } while (pool_create == POOL_RESET);

    pid++;

    /* Security check */
    if (pid == m->layer.init.pools) pool_create = POOL_FINALIZE;

    /* Revert to normal mode, after the restarted pool is finished */
    if (m->mode == RESTART_MODE && pool_create == POOL_CREATE_NEW) m->mode = NORMAL_MODE;
  } while (pool_create != POOL_FINALIZE);

  /* Finalize the pool bank */
  for (pid = 0; pid < m->layer.init.pools; pid++) {
    PoolFinalize(m, p[pid]);
  }
  free(p);

  /* Process the simulation */
  Process(m);

  return mstat;
}
