/**
 * @file
 * The Task Farm
 */
#include "Taskfarm.h"

/**
 * @function
 */
int Taskfarm(module *m) {
  int mstat = 0;
  int pid = 0;
  pool **p = NULL;
  int pool_create;

  Prepare(m);

  /**
   * Initialize the pool bank
   */
  p = calloc(m->layer.init.pools * sizeof(pool*), sizeof(pool*));
  if (!p) Error(CORE_ERR_MEM);

  for (pid = 0; pid < m->layer.init.pools; pid++) {
    p[pid] = PoolLoad(m, pid);
  }

  /**
   * The Pool loop
   */
  pid = 0; // reset pid
  pool_create = POOL_CREATE_NEW;
  do {

    /* Pool storage */
    Storage(m, p[pid]);

    do {
      mstat = PoolReset(m, p[pid]);
      CheckStatus(mstat);

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

  } while (pool_create != POOL_FINALIZE);

  /* Finalize the pool bank */
  for (pid = 0; pid < m->layer.init.pools; pid++) {
    PoolFinalize(m, p[pid]);
  }
  free(p);

  Process(m);

  return mstat;
}
