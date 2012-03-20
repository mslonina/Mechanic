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
  int pid = 0, node;
  pool p;

  node = m->node;

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
    
    if (node == MASTER) {
      mstat = Master(m, &p);
    } else {
      mstat = Worker(m, &p);
    }

    mstat = PoolProcess(m, &p); 
    PoolFinalize(m, &p);

    pid++;
    if (mstat == POOL_FINALIZE) break;
  } /* The Pool loop */

  if (node == MASTER) {
    H5Gclose(m->location);
    H5Fclose(m->datafile);
  }


  return mstat;
}
