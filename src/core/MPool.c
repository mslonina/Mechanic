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
  int i = 0, j = 0;
  pool *p = NULL;

  /* Allocate pool pointer */
  p = calloc(1, sizeof(pool));
  if (!p) Error(CORE_ERR_MEM);

  /* Allocate pool data banks */
  p->storage = calloc(m->layer.init.banks_per_pool, sizeof(storage));
  if (!p->storage) Error(CORE_ERR_MEM);

  /* Pool dataset attributes */
  for (i = 0; i < m->layer.init.banks_per_pool; i++) {
    p->storage[i].layout = (schema) STORAGE_END;
    p->storage[i].memory = NULL;
    p->storage[i].attr = calloc(m->layer.init.attr_per_dataset, sizeof(storage));
    if (!p->storage[i].attr) Error(CORE_ERR_MEM);
    for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
      p->storage[i].attr[j].layout = (schema) ATTR_STORAGE_END;
      p->storage[i].attr[j].memory = NULL;
    }
  }

  /* Allocate task board pointer */
  p->board = calloc(1, sizeof(storage));
  if (!p->board) Error(CORE_ERR_MEM);

  p->board->layout = (schema) STORAGE_END;
  p->board->memory = NULL;

  /* Task board attributes */
  p->board->attr = calloc(m->layer.init.attr_per_dataset, sizeof(storage));
  if (!p->board->attr) Error(CORE_ERR_MEM);
  for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
    p->board->attr[j].layout = (schema) ATTR_STORAGE_END;
    p->board->attr[j].memory = NULL;
  }

  /* Allocate task pointer */
  p->task = calloc(1, sizeof(task));
  if (!p->task) Error(CORE_ERR_MEM);

  p->task->storage = calloc(m->layer.init.banks_per_task, sizeof(storage));
  if (!p->task->storage) Error(CORE_ERR_MEM);

  /* Task dataset attributes */
  for (i = 0; i < m->layer.init.banks_per_task; i++) {
    p->task->storage[i].layout = (schema) STORAGE_END;
    p->task->storage[i].memory = NULL;
    p->task->storage[i].attr = calloc(m->layer.init.attr_per_dataset, sizeof(storage));
    if (!p->task->storage[i].attr) Error(CORE_ERR_MEM);
    for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
      p->task->storage[i].attr[j].layout = (schema) ATTR_STORAGE_END;
      p->task->storage[i].attr[j].memory = NULL;
    }
  }

  p->tasks = NULL;

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
  int mstat = SUCCESS, i = 0;
  double setup_attr;
  query *q;
  setup *s = &(m->layer.setup);

  if (m->node == MASTER) {

    /**
     * Assign default attributes
     *
     * The user can overwrite them in PoolPrepare() hook
     *
     * @todo: Remove when all options will be stored as attributes by default 
     */
    setup_attr = LRC_option2double("core", "xmin", s->head);
    WriteAttr(&p->board->attr[0], &setup_attr);

    setup_attr = LRC_option2double("core", "xmax", s->head);
    WriteAttr(&p->board->attr[1], &setup_attr);

    setup_attr = LRC_option2double("core", "ymin", s->head);
    WriteAttr(&p->board->attr[2], &setup_attr);
    
    setup_attr = LRC_option2double("core", "ymax", s->head);
    WriteAttr(&p->board->attr[3], &setup_attr);
    
    setup_attr = LRC_option2double("core", "zmin", s->head);
    WriteAttr(&p->board->attr[4], &setup_attr);
    
    setup_attr = LRC_option2double("core", "zmax", s->head);
    WriteAttr(&p->board->attr[5], &setup_attr);
    
    setup_attr = LRC_option2double("core", "xorigin", s->head);
    WriteAttr(&p->board->attr[6], &setup_attr);
    
    setup_attr = LRC_option2double("core", "yorigin", s->head);
    WriteAttr(&p->board->attr[7], &setup_attr);
    
    setup_attr = LRC_option2double("core", "zorigin", s->head);
    WriteAttr(&p->board->attr[8], &setup_attr);

    q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    if (q) mstat = q(all, p, s);
    CheckStatus(mstat);

    p->state = POOL_PREPARED;

    mstat = PoolProcessData(m, p, s);

  }

  /* Broadcast pool data */
  for (i = 0; i < m->pool_banks; i++) {
    if (p->storage[i].layout.sync) {
      if ((int)p->storage[i].layout.elements > 0) {
        MPI_Bcast(&(p->storage[i].memory[0]), p->storage[i].layout.elements,
            p->storage[i].layout.mpi_datatype, MASTER, MPI_COMM_WORLD);
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

  if (m->node == MASTER) {
    q = LoadSym(m, "PoolProcess", LOAD_DEFAULT);
    if (q) pool_create = q(all, p, s);

    p->state = POOL_PROCESSED;

    mstat = PoolProcessData(m, p, s);
    CheckStatus(mstat);
  }

  MPI_Bcast(&pool_create, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  p->status = pool_create;

  return pool_create;
}

/**
 * @brief Process pool data
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param s The setup pointer
 *
 * @return SUCCESS on success, error code otherwise
 */
int PoolProcessData(module *m, pool *p, setup *s) {
  int mstat = SUCCESS;
  int i = 0, j = 0, k = 0, task_groups = 0;
  char path[LRC_CONFIG_LEN];
  query *q;
  hid_t h5location, h5pool, h5tasks, h5task, h5dataset;
  hid_t attr_s, attr_d;
  hid_t hstat;

  /* Do some data processing */
  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  H5CheckStatus(h5location);

  sprintf(path, POOL_PATH, p->pid);
  h5pool = H5Gopen(h5location, path, H5P_DEFAULT);
  H5CheckStatus(h5pool);

  /* Process task board attributes */
  h5dataset = H5Dopen(h5pool, p->board->layout.name, H5P_DEFAULT);
  H5CheckStatus(h5dataset);

  for (j = 0; j < p->board->attr_banks; j++) {
    mstat = CommitAttribute(h5dataset, &p->board->attr[j]);
    CheckStatus(mstat);
  }

  H5Dclose(h5dataset);
  
  /* Write pool data */
  mstat = CommitData(h5pool, m->pool_banks, p->storage);
  CheckStatus(mstat);

  /* Write global attributes */
  if (H5Aexists(h5pool, "Status") > 0) {
    attr_d = H5Aopen(h5pool, "Status", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_SHORT, &p->state);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate(h5pool, "Status", H5T_NATIVE_SHORT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_DOUBLE, &p->state); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  if (H5Aexists(h5pool, "Id") > 0) {
    attr_d = H5Aopen(h5pool, "Id", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_INT, &p->pid);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate(h5pool, "Id", H5T_NATIVE_INT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_DOUBLE, &p->pid); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  /* The last pool link */
  if (p->state == POOL_PREPARED) {
    if (H5Lexists(h5location, LAST_GROUP, H5P_DEFAULT)) {
      H5Ldelete(h5location, LAST_GROUP, H5P_DEFAULT);
    }

    hstat = H5Lcreate_hard(h5pool, path, h5location, LAST_GROUP, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(hstat);
  }

  /* Process all datasets in the current pool */
  for (i = 0; i < m->pool_banks; i++) {
    if (p->storage[i].layout.use_hdf) {
      h5dataset = H5Dopen(h5pool, p->storage[i].layout.name, H5P_DEFAULT);
      H5CheckStatus(h5dataset);

      q = LoadSym(m, "DatasetProcess", LOAD_DEFAULT);
      if (q) mstat = q(h5pool, h5dataset, p, &(p->storage[i]), s);
      CheckStatus(mstat);

      for (j = 0; j < p->storage[i].attr_banks; j++) {
        mstat = CommitAttribute(h5dataset, &p->storage[i].attr[j]);
        CheckStatus(mstat);
      }

      H5Dclose(h5dataset);
    }
  }

  h5tasks = H5Gopen(h5pool, TASKS_GROUP, H5P_DEFAULT);

  /* Process all task datasets in the current pool */
  for (i = 0; i < m->task_banks; i++) {
    if (p->task->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.storage_type != STORAGE_GROUP) {
        h5dataset = H5Dopen(h5tasks, p->task->storage[i].layout.name, H5P_DEFAULT);
        H5CheckStatus(h5dataset);

        q = LoadSym(m, "DatasetProcess", LOAD_DEFAULT);
        if (q) mstat = q(h5tasks, h5dataset, p, &(p->task->storage[i]), s);
        CheckStatus(mstat);

        for (j = 0; j < p->task->storage[i].attr_banks; j++) {
          mstat = CommitAttribute(h5dataset, &p->task->storage[i].attr[j]);
          CheckStatus(mstat);
        }

        H5Dclose(h5dataset);
      } else {
        task_groups = 1;
      }
    }
  }

  /* This may be memory heavy */
  if (task_groups) {
    for (i = 0; i < p->pool_size; i++) {
      sprintf(path, TASK_PATH, i);
      h5task = H5Gopen(h5tasks, path, H5P_DEFAULT);

      for (j = 0; j < m->task_banks; j++) {
        if (p->task->storage[j].layout.use_hdf &&
            p->task->storage[j].layout.storage_type == STORAGE_GROUP) {

          h5dataset = H5Dopen(h5task, p->task->storage[j].layout.name, H5P_DEFAULT);
          H5CheckStatus(h5dataset);
          
          q = LoadSym(m, "DatasetProcess", LOAD_DEFAULT);
          if (q) mstat = q(h5task, h5dataset, p, &(p->task->storage[j]), s);
          CheckStatus(mstat);

          for (k = 0; k < p->task->storage[j].attr_banks; k++) {
            mstat = CommitAttribute(h5dataset, &p->task->storage[j].attr[k]);
            CheckStatus(mstat);
          }

          H5Dclose(h5dataset);
        }
      }
      H5Gclose(h5task);
    }
  }

  H5Gclose(h5tasks);
  H5Gclose(h5pool);
  H5Fclose(h5location);
  
  return mstat;
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
  hid_t h5location, group;
  char path[LRC_CONFIG_LEN];
  short ****board;

  /* Reset the board memory banks */
  if (m->node == MASTER) {
    board = AllocateShort4D(p->board);
    memset(&board[0][0][0][0], TASK_AVAILABLE, p->board->layout.storage_elements * sizeof(short));
    WriteData(p->board, &board[0][0][0][0]);

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

    FreeShort4D(board);
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
    for (i = 0; i < m->layer.init.banks_per_pool; i++) {
      free(p->storage[i].attr);
    }
    FreeMemoryLayout(m->pool_banks, p->storage);
    free(p->storage);
  }

  if (p->task) {
    if (p->task->storage) {
      for (i = 0; i < m->layer.init.banks_per_pool; i++) {
        free(p->task->storage[i].attr);
      }
      FreeMemoryLayout(m->task_banks, p->task->storage);
      free(p->task->storage);
    }

    free(p->task);
  }

  if (p->tasks && m->node == MASTER) {
    for (i = 0; i < p->pool_size; i++) {
      TaskFinalize(m, p, p->tasks[i]);
    }

    free(p->tasks);
  }

  if (p->board) {
    FreeMemoryLayout(1, p->board);
    free(p->board->attr);
    free(p->board);
  }

  if (p) free(p);
}

