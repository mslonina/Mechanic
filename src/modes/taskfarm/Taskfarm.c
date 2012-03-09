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
  pool p;
  int pid, node;
  query *q;
  node = m->node;

  if (node == MASTER) {
    m->datafile = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    m->location = H5Gcreate(m->datafile, "Pools", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }

  pid = 0;
  while (1) {
    p = PoolLoad(m, pid);
    mstat = PoolInit(m, &p);
    Storage(m, &p);

    mstat = PoolPrepare(m, &p);

    // The task loop goes here 

    mstat = PoolPostprocess(m, &p); 
//    printf("node %d, pool mstat%d\n", node, mstat);
    PoolFinalize(m, &p);
    pid++;
    if (mstat == POOL_FINALIZE) break;
  }

  if (node == MASTER) {
    H5Gclose(m->location);
    H5Fclose(m->datafile);
  }

  return mstat;
}
