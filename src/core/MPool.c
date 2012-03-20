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

  if (m->node == MASTER) {
    p.location = H5Gcreate(m->location, p.name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }

  p.storage = calloc(m->layer.init.banks_per_pool * sizeof(storage*), sizeof(storage*));
  if (!p.storage) Error(CORE_ERR_MEM);
  
  p.task.storage = calloc(m->layer.init.banks_per_task * sizeof(storage*), sizeof(storage*));
  if (!p.task.storage) Error(CORE_ERR_MEM);

  return p;
}

/**
 * @function
 * Initializes the task pool.
 */
int PoolInit(module *m, pool *p) {
  int mstat = 0;

  return mstat;
}

/**
 * @function
 * Prepares the pool.
 */
int PoolPrepare(module *m, pool *p) {
  int mstat = 0, i = 0;
  query *q;
  setup s = m->layer.setup;
  int size;

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    if (q) mstat = q(p, s);
    mstat = WritePoolData(p);
  }

  while (p->storage[i].layout.path) {
    if (p->storage[i].layout.sync) {
      size = GetSize(p->storage[i].layout.rank, p->storage[i].layout.dim);
      MPI_Bcast(&(p->storage[i].data[0][0]), size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    }
    i++;
  }

  return mstat;
}

/**
 * @function
 * Processes the pool.
 */
int PoolProcess(module *m, pool *p) {
  int mstat = 0;
  setup s = m->layer.setup;
  query *q;

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolProcess", LOAD_DEFAULT);
    if (q) mstat = q(p, s);
  }

  MPI_Bcast(&mstat, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  return mstat;
}

/**
 * @function
 * Finalizes the pool
 */
void PoolFinalize(module *m, pool *p) {
  FreeDoubleArray(p->storage->data, p->storage->layout.dim);
  free(p->storage);
  free(p->task.storage);
  if (m->node == MASTER) H5Gclose(p->location);
}
