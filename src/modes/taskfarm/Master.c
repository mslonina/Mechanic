/**
 * @file
 * The Master node 
 */
#include "Taskfarm.h"

int Master(module *m) {
  int mstat = 0;
  pool p;
  int pid;
  hid_t data_group;
  query *q;

  pid = 0;

  m->datafile = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  data_group = H5Gcreate(m->datafile, "Pools", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  while (1) {
    p = PoolLoad(m, pid, data_group);
    mstat = PoolInit(m, &p);

    /**
     * @todo Pool Prepare
     *
     * - Load any data specified in the PoolPrepare function
     * - Broadcast all neccessary information
     */
    //q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    //if (q) mstat = q(&p);

    /**
     * @todo The Task Loop goes here
     */

    q = LoadSym(m, "PoolPostprocess", LOAD_DEFAULT);
    if (q) mstat = q(&p);
    PoolFinalize(&p);
    pid++;
    if (mstat == POOL_FINALIZE) break;
  } 

  H5Gclose(data_group);
  H5Fclose(m->datafile);

  return mstat;
}
