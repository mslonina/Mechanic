/**
 * @file
 * The Task Pool related functions
 */
#include "MPool.h"

/**
 * @brief Load the task pool
 *
 * @param m The module pointer
 * @param pid The pool ID
 *
 * @return The pool pointer, NULL otherwise
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
 * @brief Prepare the pool
 *
 * @param m The module pointer
 * @param all The pointer to pool array (all pools)
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int PoolPrepare(module *m, pool **all, pool *p) {
  int mstat = SUCCESS, i = 0, size = 0, task_groups = 0;
  query *q;
  setup *s = &(m->layer.setup);
  hid_t h5location, group;
  hid_t hstat;
  hsize_t adims;
  int attr_data[1];
  char path[LRC_CONFIG_LEN];

  /* Number of tasks to do */
  if (m->node == MASTER) {
    p->pool_size = GetSize(p->board->layout.rank, p->board->layout.dim);
  }
  MPI_Bcast(&(p->pool_size), 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  /* Allocate the tasks group,
   * We cannot do it earlier, since we don't know the size of the pool */
  if (m->node == MASTER) {
    for (i = 0; i < m->task_banks; i++) {
      if (p->task->storage[i].layout.storage_type == STORAGE_GROUP) task_groups = 1;
    }

    /* FIX IT! THIS PART IS TRAGIC FOR MEMORY ALLOCATION IN HUGE RUNS */
    if (task_groups) {
      p->tasks = calloc(p->pool_size * sizeof(task*), sizeof(task*));
      for (i = 0; i < p->pool_size; i++) {
        p->tasks[i] = TaskLoad(m, p, i);
      }
    }

    q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    if (q) mstat = q(all, p, s);
    CheckStatus(mstat);

    /* Write pool data */
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    H5CheckStatus(h5location);

    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);
    H5CheckStatus(group);

    mstat = CommitData(group, m->pool_banks, p->storage);
    CheckStatus(mstat);

    /* Create "latest" link */
    if (H5Lexists(h5location, LAST_GROUP, H5P_DEFAULT)) {
      H5Ldelete(h5location, LAST_GROUP, H5P_DEFAULT);
    }
    hstat = H5Lcreate_hard(group, path, h5location, LAST_GROUP, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(hstat);

    /* Create attributes */
    if (p->rid == 0 && m->mode != RESTART_MODE) {
      adims = 1;
      attr_data[0] = p->pid;
      hstat = H5LTset_attribute_int(group, "board", "Id", attr_data, adims);
      H5CheckStatus(hstat);
      
      attr_data[0] = 0;
      hstat = H5LTset_attribute_int(group, "board", "Status", attr_data, adims);
      H5CheckStatus(hstat);
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
 * @brief Process the pool
 *
 * @param m The module pointer
 * @param all The pointer to pool array (all pools)
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int PoolProcess(module *m, pool **all, pool *p) {
  int mstat = SUCCESS;
  int pool_create = 0;
  setup *s = &(m->layer.setup);
  query *q;
  hid_t h5location, group;
  hid_t hstat;
  hsize_t adims;
  char path[LRC_CONFIG_LEN];
  int attr_data[1];

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolProcess", LOAD_DEFAULT);
    if (q) pool_create = q(all, p, s);

    /* Write pool data */
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    H5CheckStatus(h5location);

    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);
    H5CheckStatus(group);

    mstat = CommitData(group, m->pool_banks, p->storage);
    CheckStatus(mstat);

    /* Write attributes */
    adims = 1;
    attr_data[0] = 1;

    hstat = H5LTset_attribute_int(group, "board", "Status", attr_data, adims);
    H5CheckStatus(hstat);

    H5Gclose(group);
    H5Fclose(h5location);
  }

  MPI_Bcast(&pool_create, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  return pool_create;
}

/**
 * @brief Reset the current pool
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int PoolReset(module *m, pool *p) {
  int mstat = SUCCESS;
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
    H5CheckStatus(h5location);

    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen(h5location, path, H5P_DEFAULT);
    H5CheckStatus(group);

    mstat = CommitData(group, 1, p->board);
    CheckStatus(mstat);

    H5Gclose(group);
    H5Fclose(h5location);
  }

  return mstat;
}
/**
 * @brief Finalize the pool
 *
 * @param m The module pointer
 * @param p The pool pointer to finalize
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

  if (p->tasks && m->node == MASTER) {
    for (i = 0; i < p->pool_size; i++) {
      TaskFinalize(m, p, p->tasks[i]);
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

