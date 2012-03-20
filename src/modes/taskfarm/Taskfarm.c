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

  if (m->node == MASTER) {
    m->datafile = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    m->location = H5Gcreate(m->datafile, "Pools", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }

  /**
   * The Pool loop
   */
  pid = 0;
  pool_create = POOL_CREATE_NEW;
  do {
    p = PoolLoad(m, pid);
    mstat = PoolInit(m, p);
    CheckStatus(mstat);

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
    
    //if (m->node == MASTER) {
    //  mstat = Master(m, p);
    //} else {
    //  mstat = Worker(m, p);
    //}

    pool_create = PoolProcess(m, p); 
    
    PoolFinalize(m, p);
    pid++;
  } while (pool_create != POOL_FINALIZE); 
  /* The Pool loop */

  if (m->node == MASTER) {
    H5Gclose(m->location);
    H5Fclose(m->datafile);
  }

  return mstat;
}
