/**
 * @file
 * The Task Pool related functions.
 */
#include "MPool.h"

/**
 * @function
 * Loads the task pool
 */
pool PoolLoad(module *m, int pid) {
  pool p;
  
  p.pid = pid;
  sprintf(p.name,"pool-%04d",pid);

  p.location = H5Gcreate(m->location, p.name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  return p;
}

/**
 * @function
 * Initializes the task pool.
 */
int PoolInit(module *m, pool *p) {
  int mstat = 0;

  p->storage = calloc(m->layer.init.banks_per_pool * sizeof(storage), sizeof(storage));
  if (!p->storage) Error(CORE_ERR_MEM);
  
  p->task.storage = calloc(m->layer.init.banks_per_task * sizeof(storage), sizeof(storage));
  if (!p->task.storage) Error(CORE_ERR_MEM);

  return mstat;
}

/**
 * @function
 * Finalizes the pool
 */
void PoolFinalize(pool *p) {
  FreeDoubleArray(p->storage->data, p->storage->layout.dim);
  free(p->storage);
  free(p->task.storage);
  H5Gclose(p->location);
}
