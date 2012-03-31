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

    if (m->node == MASTER) Message(MESSAGE_INFO, "Running the task pool %d ...\n", p[pid]->pid);

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
   
    if (m->node == MASTER) Message(MESSAGE_CONT, "Pool %d finished.\n", p[pid]->pid);
    
    pid++;
  } while (pool_create != POOL_FINALIZE);

  /* Finalize the pool bank */
  for (pid = 0; pid < m->layer.init.pools; pid++) {  
    PoolFinalize(m, p[pid]);
  }
  free(p);

  return mstat;
}
