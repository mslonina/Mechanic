/**
 * @file
 */
#include "MPool.h"

/**
 * @function
 * Loads the task pool
 */
pool PoolLoad(module *m, int pid, hid_t location) {
  pool p;
  
  p.pid = pid;
  sprintf(p.name,"pool-%04d",pid);

  p.location = H5Gcreate(location, p.name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  return p;
}

/**
 * @function
 * Initializes the task pool, its memory and storage layout
 */
int PoolInit(module *m, pool *p) {
  int mstat = 0;
  query *q;

  p->storage = calloc(m->layer.init.datasets_per_pool * sizeof(storage), sizeof(storage));
  if (!p->storage) Error(CORE_ERR_MEM);

  /* First load the fallback (core) storage layout */
  if (m->fallback.handler) {
    q = LoadSym(m, "Storage", FALLBACK_ONLY);
    if (q) mstat = q(p, &m->layer.setup);
    CheckStatus(mstat);
  }

  /* Load the module setup */
  q = LoadSym(m, "Storage", NO_FALLBACK);
  if (q) mstat = q(p, &m->layer.setup);
  CheckStatus(mstat);
  
  /* Commit the storage layout */
  CommitStorageLayout(p->location, p->storage);

  /**
   * @todo Commit the memory layout
   */
  return mstat;
}

/**
 * @function
 * Finalizes the pool
 */
void PoolFinalize(pool *p) {
  free(p->storage);
  H5Gclose(p->location);
}
