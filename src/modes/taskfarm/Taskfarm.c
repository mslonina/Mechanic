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
  pool *p = NULL;
  int pool_create;

  /**
   * The Pool loop
   */
  pid = 0;
  pool_create = POOL_CREATE_NEW;
  while (pool_create != POOL_FINALIZE) {
    p = PoolLoad(m, pid);
   // mstat = PoolInit(m, p);
   // CheckStatus(mstat);

    mstat = Storage(m, p);
    CheckStatus(mstat);

    mstat = PoolPrepare(m, p);
    CheckStatus(mstat);

    /**
     * The Task loop
     *
     * @todo
     * - Implement the task storage scheme
     * - Implement the task board
     */
    
    if (m->node == MASTER) {
      mstat = Master(m, p);
    } else {
      mstat = Worker(m, p);
    }

    pool_create = PoolProcess(m, p); 
    
    PoolFinalize(m, p);
    pid++;
  } 
  /* The Pool loop */

  return mstat;
}
