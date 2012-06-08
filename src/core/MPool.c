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

  p->pid = pid;
  p->rid = 0;
  p->node = m->node;
  p->mpi_size = m->mpi_size;

  return p;
}

/**
 * @function
 * Prepare the pool
 */
int PoolPrepare(module *m, pool **all, pool *p) {
  int mstat = 0, i = 0, size = 0;
  query *q;
  setup *s = &(m->layer.setup);
  hid_t h5location, group, attribute_id, attrspace_id;
  hsize_t adims;
  int attr_id;
  char path[LRC_CONFIG_LEN];

  /* Number of tasks to do */
  p->pool_size = GetSize(p->board->layout.rank, p->board->layout.dim);

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    if (q) mstat = q(all, p, s);

    /* Write pool data */
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);
    mstat = CommitData(group, m->pool_banks, p->storage);

    /* Create "latest" link */
    if (H5Lexists(h5location, LAST_GROUP, H5P_DEFAULT)) {
      H5Ldelete(h5location, LAST_GROUP, H5P_DEFAULT);
    }
    H5Lcreate_hard(group, path, h5location, LAST_GROUP, H5P_DEFAULT, H5P_DEFAULT);

    /* Create attributes */
    if (p->rid == 0 && m->mode != RESTART_MODE) {
      adims = 1;
      attr_id = p->pid;
      attrspace_id = H5Screate_simple(1, &adims, NULL);
      attribute_id = H5Acreate(group, "Id", H5T_NATIVE_INT, attrspace_id, H5P_DEFAULT, H5P_DEFAULT);
      H5Awrite(attribute_id, H5T_NATIVE_INT, &attr_id);
      H5Aclose(attribute_id);
      H5Sclose(attrspace_id);
    }

    H5Gclose(group);
    H5Fclose(h5location);
  }

  /* Broadcast pool data */
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
int PoolProcess(module *m, pool **all, pool *p) {
  int pool_create = 0;
  setup *s = &(m->layer.setup);
  query *q;
  hid_t h5location, group;
  char path[LRC_CONFIG_LEN];

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolProcess", LOAD_DEFAULT);
    if (q) pool_create = q(all, p, s);

    /* Write pool data */
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);

    CommitData(group, m->pool_banks, p->storage);

    H5Gclose(group);
    H5Fclose(h5location);

  }

  MPI_Bcast(&pool_create, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  return pool_create;
}

/**
 * @function
 * Reset the pool
 */
int PoolReset(module *m, pool *p) {
  int mstat = 0;
  int i,j;
  hid_t h5location, group;
  char path[LRC_CONFIG_LEN];

  /* Reset the board memory banks */
  if (m->node == MASTER) {
    for (i = 0; i < p->board->layout.dim[0]; i++) {
      for (j = 0; j < p->board->layout.dim[1]; j++) {
        p->board->data[i][j] = TASK_AVAILABLE;
      }
    }

    /* Reset the board storage banks */
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);
    mstat = CommitData(group, 1, p->board);

    H5Gclose(group);
    H5Fclose(h5location);
  }

  return mstat;
}
/**
 * @function
 * Finalize the pool
 */
void PoolFinalize(module *m, pool *p) {
  int i = 0;
  if (p->storage) {
    if (p->storage->data) {
      FreeBuffer(p->storage->data);
    }
    free(p->storage);
  }

  if (p->task) {
    for (i = 0; i < m->task_banks; i++) {
      if (p->task->storage[i].data) {
        FreeBuffer(p->task->storage[i].data);
      }
    }
    if (p->task->storage) free(p->task->storage);
    free(p->task);
  }

  if (p->tasks) {
    for (i = 0; i < p->pool_size; i++) {
      TaskFinalize(m, p, &(p->tasks[i]));
    }
    free(p->tasks);
  }

  if (p->board) {
    if (p->board->data) {
      FreeBuffer(p->board->data);
    }
    free(p->board);
  }

  if (p) free(p);
}
