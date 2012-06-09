/**
 * @file
 * The storage stage
 */
#include "MStorage.h"

/**
 * @function
 * @brief Create the storage layout for the pool
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int Storage(module *m, pool *p) {
  int mstat = 0;
  int i, j, dims[MAX_RANK];
  int task_groups = 0;
  query *q;

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

  /* The pool size */
  p->pool_size = GetSize(p->board->layout.rank, p->board->layout.dim);

  /* Memory/Storage banks */
  m->pool_banks = GetBanks(m->layer.init.banks_per_pool, p->storage);
  m->task_banks = GetBanks(m->layer.init.banks_per_task, p->task->storage);

  /* Check and fix the memory/storage layout */
  CheckLayout(1, p->board);
  CheckLayout(m->pool_banks, p->storage);

  /* Right now, there is no support for different storage types of pool datasets */
  for (i = 0; i < m->pool_banks; i++) {
    p->storage[i].layout.storage_type = STORAGE_BASIC;
  }

  CheckLayout(m->task_banks, p->task->storage);

  /* Commit memory layout used during the task pool */
  CommitMemoryLayout(m->pool_banks, p->storage);

  /* Commit memory for task banks (whole datasets) */
  for (i = 0; i < m->task_banks; i++) {
    if (p->task->storage[i].layout.storage_type == STORAGE_BASIC) task_groups = 1;
    if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
      p->task->storage[i].layout.storage_type == STORAGE_LIST) {
      dims[0] = p->task->storage[i].layout.dim[0] * p->pool_size;
      dims[1] = p->task->storage[i].layout.dim[1];
      p->task->storage[i].data = AllocateBuffer(p->task->storage[i].layout.rank, dims);
    }
    if (p->task->storage[i].layout.storage_type == STORAGE_BOARD) {
      dims[0] = p->task->storage[i].layout.dim[0] * p->board->layout.dim[0];
      dims[1] = p->task->storage[i].layout.dim[1] * p->board->layout.dim[1];
      p->task->storage[i].data = AllocateBuffer(p->task->storage[i].layout.rank, dims);
    }
    p->task->storage[i].layout.offset[0] = 0;
    p->task->storage[i].layout.offset[1] = 0;
  }

  /* Commit the storage layout (only the Master node) */
  if (m->node == MASTER && m->mode != RESTART_MODE) {
    CommitStorageLayout(m, p);
  }

  /* Commit Board */
  p->board->data = AllocateBuffer(p->board->layout.rank, p->board->layout.dim);
  if (m->mode != RESTART_MODE) {
    for (i = 0; i < p->board->layout.dim[0]; i++) {
      for (j = 0; j < p->board->layout.dim[1]; j++) {
        p->board->data[i][j] = TASK_AVAILABLE;
      }
    }
  }

  /* TaskID groups */
  if (task_groups) {
    p->tasks = calloc(p->pool_size * sizeof(task*), sizeof(task*));
    for (i = 0; i < p->pool_size; i++) {
      p->tasks[i] = *TaskLoad(m, p, i);
    }
  }

  return mstat;
}

/**
 * @function
 * @brief Check for any inconsistencies in the storage layout
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure to check
 *
 * @return 0 on success, error code otherwise
 */
int CheckLayout(int banks, storage *s) {
  int i = 0, mstat = 0;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.rank <= 0) {
      Message(MESSAGE_ERR, "Rank must be > 0\n");
      Error(CORE_ERR_STORAGE);
    }

    s[i].layout.rank = MAX_RANK;
    s[i].layout.datatype = H5T_NATIVE_DOUBLE;

    if (s[i].layout.use_hdf) {
      /* Common fixes before future development */
      s[i].layout.dataspace_type = H5S_SIMPLE;
      s[i].layout.offset[0] = 0; // Offsets calculated automatically
      s[i].layout.offset[1] = 0;

      /* Check for mistakes */
      if (s[i].layout.path == NULL) {
        Message(MESSAGE_ERR, "The storage path is required when use_hdf\n");
        Error(CORE_ERR_STORAGE);
      }
      if (s[i].layout.storage_type < 0) {
        Message(MESSAGE_ERR, "The storage type is missing\n");
        Error(CORE_ERR_STORAGE);
      }
    }
  }

  return mstat;
}

/**
 * @function
 * @brief Commits the memory layout
 *
 * This function must run on every node -- the size of data arrays shared between nodes
 * depends on the memory layout.
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 *
 */
int CommitMemoryLayout(int banks, storage *s) {
  int mstat = 0, i = 0;

  for (i = 0; i < banks; i++) {
    s[i].data = AllocateBuffer(s[i].layout.rank, s[i].layout.dim);
  }

  return mstat;
}

/**
 * @function
 * @brief Frees the memory layout
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure to free
 */
void FreeMemoryLayout(int banks, storage *s) {
  int i = 0;

  for (i = 0; i < banks; i++) {
    if (s[i].data) {
      FreeBuffer(s[i].data);
    }
  }
}

/**
 * @function
 * @brief Commit the storage layout
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int CommitStorageLayout(module *m, pool *p) {
  int mstat = 0, i = 0, j = 0;
  hid_t h5location, h5group, h5pools, h5tasks, h5task;
  char path[LRC_CONFIG_LEN];

  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);

  /* The Pools */
  if (!H5Lexists(h5location, POOLS_GROUP, H5P_DEFAULT)) {
    h5pools = H5Gcreate(h5location, POOLS_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  } else {
    h5pools = H5Gopen(h5location, POOLS_GROUP, H5P_DEFAULT);
  }

  /* The Pool-ID group */
  sprintf(path, POOL_PATH, p->pid);
  h5group = H5Gcreate(h5pools, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* The Pool-ID task board */
  CreateDataset(h5group, p->board, m, p);

  /* The Pool-ID datasets */
  for (i = 0; i < m->pool_banks; i++) {
    if (p->storage[i].layout.use_hdf) {
      CreateDataset(h5group, &p->storage[i], m, p);
    }
  }

  /* The Tasks group */
  h5tasks = H5Gcreate(h5group, TASKS_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* The task datasets */
  for (i = 0; i < m->task_banks; i++) {
    if (p->task->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[i].layout.storage_type == STORAGE_LIST ||
        p->task->storage[i].layout.storage_type == STORAGE_BOARD) {
          CreateDataset(h5tasks, &p->task->storage[i], m, p);
      }
      if (p->task->storage[i].layout.storage_type == STORAGE_BASIC) {
        for (j = 0; j < p->pool_size; j++) {
          sprintf(path, TASK_PATH, j);
          if (!H5Lexists(h5group, path, H5P_DEFAULT)) {
            h5task = H5Gcreate(h5tasks, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
          } else {
            h5task = H5Gopen(h5tasks, path, H5P_DEFAULT);
          }
          CreateDataset(h5task, &p->task->storage[i], m, p);
          H5Gclose(h5task);
        }
      }
    }
  }

  H5Gclose(h5tasks);
  H5Gclose(h5group);
  H5Gclose(h5pools);
  H5Fclose(h5location);

  return mstat;
}

/**
 * @function
 * @brief Create a dataset
 *
 * @param h5location The HDF5 location id
 * @param s The storage structure
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int CreateDataset(hid_t h5location, storage *s, module *m, pool *p) {
  int mstat = 0;
  hid_t h5dataset, h5dataspace;
  hsize_t dims[MAX_RANK];
  herr_t h5status;

  h5dataspace = H5Screate(s->layout.dataspace_type);
  if (s->layout.dataspace_type == H5S_SIMPLE) {
    if (s->layout.storage_type == STORAGE_BASIC) {
      dims[0] = s->layout.dim[0];
      dims[1] = s->layout.dim[1];
    }
    if (s->layout.storage_type == STORAGE_PM3D ||
        s->layout.storage_type == STORAGE_LIST) {
      dims[0] = s->layout.dim[0] * p->pool_size;
      dims[1] = s->layout.dim[1];
    }
    if (s->layout.storage_type == STORAGE_BOARD) {
      dims[0] = s->layout.dim[0] * p->board->layout.dim[0];
      dims[1] = s->layout.dim[1] * p->board->layout.dim[1];
    }
    h5status = H5Sset_extent_simple(h5dataspace, s->layout.rank, dims, NULL);
    if (h5status < 0) return CORE_ERR_HDF;
  }

  h5dataset = H5Dcreate(h5location, s->layout.path, s->layout.datatype, h5dataspace,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  H5Dclose(h5dataset);
  H5Sclose(h5dataspace);

  return mstat;
}

/**
 * @function
 * @brief Get the number of used memory banks
 *
 * @param allocated_banks Number of allocated memory/storage banks
 * @param s The storage structure
 *
 * @return The number of memory/storage banks in use, 0 otherwise
 */
int GetBanks(int allocated_banks, storage *s) {
  int banks_in_use = 0;
  int i = 0;

  for (i = 0; i < allocated_banks; i++) {
    if (s[i].layout.rank > 0) {
      banks_in_use++;
    }
  }

  return banks_in_use;
}

/**
 * @function
 * @brief Commit the data to the master file
 *
 * @param h5location The HDF5 location id
 * @param banks Number of memory banks to store
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 */
int CommitData(hid_t h5location, int banks, storage *s) {
  int mstat = 0, i = 0;
  hid_t dataspace, dataset, memspace;
  herr_t hdf_status = 0;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {

      dataset = H5Dopen(h5location, s[i].layout.path, H5P_DEFAULT);
      dataspace = H5Dget_space(dataset);

      /* Whole dataset at once */
      if (s[i].layout.storage_type == STORAGE_BASIC) {
        hdf_status = H5Dwrite(dataset, s[i].layout.datatype,
            H5S_ALL, H5S_ALL, H5P_DEFAULT, &(s[i].data[0][0]));
        if (hdf_status < 0) mstat = CORE_ERR_HDF;
      } else {
        dims[0] = s[i].layout.dim[0];
        dims[1] = s[i].layout.dim[1];
        offsets[0] = s[i].layout.offset[0];
        offsets[1] = s[i].layout.offset[1];
        memspace = H5Screate_simple(s[i].layout.rank, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offsets, NULL, dims, NULL);
        hdf_status = H5Dwrite(dataset, s[i].layout.datatype,
            memspace, dataspace, H5P_DEFAULT, &(s[i].data[0][0]));
        if (hdf_status < 0) mstat = CORE_ERR_HDF;

        H5Sclose(memspace);
      }

      H5Sclose(dataspace);
      H5Dclose(dataset);

    }
  }

  if (hdf_status < 0) mstat = CORE_ERR_HDF;

  return mstat;
}

/**
 * @function
 * @brief Read the dataset data
 *
 * @param h5location The HDF5 location id
 * @param banks Number of memory banks to read
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 */
int ReadData(hid_t h5location, int banks, storage *s) {
  int mstat = 0, i = 0;
  hid_t dataset;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {
      Message(MESSAGE_DEBUG, "Read Data storage path: %s\n", s[i].layout.path);
      dataset = H5Dopen(h5location, s[i].layout.path, H5P_DEFAULT);
      H5Dread(dataset, s[i].layout.datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(s[i].data[0][0]));
      H5Dclose(dataset);
    }
  }

  return mstat;
}

