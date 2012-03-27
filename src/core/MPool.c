/**
 * @file
 * The Task Pool related functions
 */
#include "MPool.h"

/**
 * @function
 * Load the task pool
 */
pool* PoolLoad(module *m, int pid) {
  pool *p = NULL;
  int i = 0;

  /* Allocate pool pointer */
  p = calloc(sizeof(pool), sizeof(pool));
  if (!p) Error(CORE_ERR_MEM);

  /* Allocate pool data banks */
  p->storage = calloc(m->layer.init.banks_per_pool * sizeof(storage), sizeof(storage));
  if (!p->storage) Error(CORE_ERR_MEM);

  for (i = 0; i < m->layer.init.banks_per_pool; i++) {
    p->storage[i].layout = (schema) STORAGE_END;
    p->storage[i].data = NULL;
  }

  /* Allocate task board pointer */
  p->board = calloc(sizeof(storage), sizeof(storage));
  if (!p->board) Error(CORE_ERR_MEM);
  p->board->layout = (schema) STORAGE_END;
  p->board->data = NULL;

  /* Allocate task pointer */
  p->task = calloc(sizeof(task), sizeof(task));
  if (!p->task) Error(CORE_ERR_MEM);

  p->task->storage = calloc(m->layer.init.banks_per_task * sizeof(storage), sizeof(storage));
  if (!p->task->storage) Error(CORE_ERR_MEM);

  for (i = 0; i < m->layer.init.banks_per_task; i++) {
    p->task->storage[i].layout = (schema) STORAGE_END;
    p->task->storage[i].data = NULL;
  }

  /* Setup some sane defaults */
  p->pid = pid;
  p->node = m->node;
  p->mpi_size = m->mpi_size;

  /* Pool storage */
  Storage(m, p);

  return p;
}

/**
 * @function
 * Initializes the task pool
 */
int PoolInit(module *m, pool *p) {
  int mstat = 0;

  return mstat;
}

/**
 * @function
 * Prepare the pool
 */
int PoolPrepare(module *m, pool *p) {
  int mstat = 0, i = 0, size = 0;
  query *q;
  setup *s = &(m->layer.setup);
  hid_t h5location, group;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];
  char path[LRC_CONFIG_LEN];

  /* Number of tasks to do */
  p->pool_size = GetSize(p->board->layout.rank, p->board->layout.dim);

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    if (q) mstat = q(p, s);
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    sprintf(path, "/Pools/pool-%04d", p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);
    mstat = CommitData(group, m->pool_banks, p->storage, STORAGE_BASIC, dims, offsets);
    H5Gclose(group);
    H5Fclose(h5location);
  }

  for (i = 0; i < m->pool_banks; i++) {
    if (p->storage[i].layout.sync) {
      size = GetSize(p->storage[i].layout.rank, p->storage[i].layout.dim);
      if (size > 0) {
        MPI_Bcast(&(p->storage[i].data[0][0]), size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
      }
    }
  }

  return mstat;
}

/**
 * @function
 * Process the pool
 */
int PoolProcess(module *m, pool *p) {
  int pool_create = 0;
  setup *s = &(m->layer.setup);
  query *q;

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolProcess", LOAD_DEFAULT);
    if (q) pool_create = q(p, s);
  }

  MPI_Bcast(&pool_create, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  return pool_create;
}

/**
 * @function
 * Finalize the pool
 */
void PoolFinalize(module *m, pool *p) {
  if (p->storage) {
    if (p->storage->data) {
      FreeBuffer(p->storage->data, p->storage->layout.dim);
    }
    free(p->storage);
  }
  if (p->task) {
    if (p->task->storage) free(p->task->storage);
    free(p->task);
  }
  //if (m->node == MASTER) H5Gclose(p->h5location);

  if (p->board) {
    if (p->board->data) {
      FreeBuffer(p->board->data, p->board->layout.dim);
    }
    free(p->board);
  }
  free(p);
}
