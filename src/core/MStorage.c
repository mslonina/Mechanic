/**
 * @file
 * The storage stage
 */
#include "MStorage.h"

/**
 * @brief Create the storage layout for the pool
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int Storage(module *m, pool *p) {
  int mstat = SUCCESS;
  int i, j, k, task_groups, size;
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

  /* Memory/Storage banks */
  m->pool_banks = GetBanks(m->layer.init.banks_per_pool, p->storage);
  m->task_banks = GetBanks(m->layer.init.banks_per_task, p->task->storage);

  for (i = 0; i < m->pool_banks; i++) {
    p->storage[i].attr_banks = 0;
    for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
      if (p->storage[i].attr[j].layout.dataspace == H5S_SIMPLE ||
          p->storage[i].attr[j].layout.dataspace == H5S_SCALAR) {
        p->storage[i].attr_banks++;
      }
    }
  }

  for (i = 0; i < m->task_banks; i++) {
    p->task->storage[i].attr_banks = 0;
    for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
      if (p->task->storage[i].attr[j].layout.dataspace == H5S_SIMPLE ||
          p->task->storage[i].attr[j].layout.dataspace == H5S_SCALAR) {
        p->task->storage[i].attr_banks++;
      }
    }
  }

  p->board->attr_banks = 0;
  for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
    if (p->board->attr[j].layout.dataspace == H5S_SIMPLE ||
        p->board->attr[j].layout.dataspace == H5S_SCALAR) {
      p->board->attr_banks++;
    }
  }

  /* Check and fix the memory/storage layout */
  CheckLayout(m, 1, p->board);
  CheckLayout(m, m->pool_banks, p->storage);

  /* The pool size */
  p->pool_size = 1;
  for (i = 0; i < TASK_BOARD_RANK; i++) {
    p->pool_size *= p->board->layout.dim[i];
  }

  /* Right now, there is no support for different storage types of pool datasets */
  for (i = 0; i < m->pool_banks; i++) {
    p->storage[i].layout.storage_type = STORAGE_GROUP;
  }

  CheckLayout(m, m->task_banks, p->task->storage);

  /* Commit memory layout used during the task pool */
  CommitMemoryLayout(m->pool_banks, p->storage);

  /* Master only memory/storage operations */
  if (m->node == MASTER) {

    /* Commit memory for task banks (whole datasets) */
    for (i = 0; i < m->task_banks; i++) {
      if (p->task->storage[i].layout.storage_type == STORAGE_GROUP) {
        task_groups = 1;
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);
      }

      if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[i].layout.storage_type == STORAGE_LIST) {

        p->task->storage[i].layout.storage_dim[0] =
          p->task->storage[i].layout.dim[0] * p->pool_size;
        
        for (j = 1; j < MAX_RANK; j++) {
          p->task->storage[i].layout.storage_dim[j] =
            p->task->storage[i].layout.dim[j];
        }
        
        size = GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.storage_dim);
        p->task->storage[i].layout.storage_elements = size;
        p->task->storage[i].layout.storage_size = (size_t)size * p->task->storage[i].layout.datatype_size;
        mstat = Allocate(&(p->task->storage[i]), (size_t)size, p->task->storage[i].layout.datatype_size);
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);
      }

      if (p->task->storage[i].layout.storage_type == STORAGE_BOARD) {
        p->task->storage[i].layout.storage_dim[0] =
          p->task->storage[i].layout.dim[0] * p->board->layout.dim[0];
        p->task->storage[i].layout.storage_dim[1] =
          p->task->storage[i].layout.dim[1] * p->board->layout.dim[1];
        p->task->storage[i].layout.storage_dim[2] =
          p->task->storage[i].layout.dim[2] * p->board->layout.dim[2];

        for (j = 3; j < MAX_RANK; j++) {
          p->task->storage[i].layout.storage_dim[j] =
            p->task->storage[i].layout.dim[j];
        }
        
        size = GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.storage_dim);
        p->task->storage[i].layout.storage_elements = size;
        p->task->storage[i].layout.storage_size = (size_t)size * p->task->storage[i].layout.datatype_size;
        mstat = Allocate(&(p->task->storage[i]), (size_t)size, p->task->storage[i].layout.datatype_size);
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);
      }

      for (j = 0; j < MAX_RANK; j++) {
        p->task->storage[i].layout.offset[j] = 0;
      }

    }

    /* @todo FIX IT! THIS PART IS TRAGIC FOR MEMORY ALLOCATION IN HUGE RUNS */
    if (task_groups) {
      p->tasks = calloc(p->pool_size, sizeof(task));
      for (i = 0; i < p->pool_size; i++) {
        p->tasks[i] = TaskLoad(m, p, i);

        /**
         * @todo Different attributes for each task
         *
         * This probably requires worker nodes to know something on attributes...
         */

      }
    }

    /* Commit Board */
    mstat = CommitMemoryLayout(1, p->board);

    /* Commit the storage layout */
    if (m->mode != RESTART_MODE) {
      CommitStorageLayout(m, p);
    }

  }

  return mstat;
}

/**
 * @brief Check for any inconsistencies in the storage layout
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure to check
 *
 * @return 0 on success, error code otherwise
 */
int CheckLayout(module *m, int banks, storage *s) {
  int i = 0, j = 0, mstat = SUCCESS;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.rank <= 1) {
      Message(MESSAGE_ERR, "Rank must be > 1\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.rank > MAX_RANK) {
      Message(MESSAGE_ERR, "Rank must be <= %d\n", MAX_RANK);
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.storage_type == STORAGE_BOARD &&
        s[i].layout.rank < TASK_BOARD_RANK) {
      Message(MESSAGE_ERR, "Minimum rank for STORAGE_BOARD is TASK_BOARD_RANK (= 3)\n");
      Error(CORE_ERR_STORAGE);
    }

    for (j = 0; j < s[i].layout.rank; j++) {
      if (s[i].layout.dim[j] <= 0) {
        Message(MESSAGE_ERR, "Invalid size for dimension %d = %d\n", i, s[i].layout.dim[j]);
        Error(CORE_ERR_STORAGE);
      }
      s[i].layout.storage_dim[j] = s[i].layout.dim[j];
    }

    if ((int)s[i].layout.datatype <= 0) {
      Message(MESSAGE_ERR, "Datatype is missing\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.storage_type < 0) {
      Message(MESSAGE_ERR, "The storage type is missing\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.use_hdf) {
      s[i].layout.dataspace = H5S_SIMPLE;
      for (j = 0; j < MAX_RANK; j++) {
        s[i].layout.offset[j] = 0; // Offsets are calculated automatically
      }

      /* Check for mistakes */
      if (s[i].layout.name == NULL) {
        Message(MESSAGE_ERR, "The storage name is required when use_hdf\n");
        Error(CORE_ERR_STORAGE);
      }
      if (s[i].layout.sync != 1) {
        Message(MESSAGE_WARN, "The sync flag for '%s' must be enabled for use_hdf. Fixing\n",
          s[i].layout.name);
        s[i].layout.sync = 1;
      }
    }

    s[i].layout.mpi_datatype = GetMpiDatatype(s[i].layout.datatype);
    s[i].layout.datatype_size = H5Tget_size(s[i].layout.datatype);
    
    s[i].layout.storage_elements = 
      s[i].layout.elements = GetSize(s[i].layout.rank, s[i].layout.dim);

    s[i].layout.storage_size = (size_t) s[i].layout.storage_elements * s[i].layout.datatype_size;
    s[i].layout.size = (size_t) s[i].layout.elements * s[i].layout.datatype_size;

    /* Check attributes */
    for (j = 0; j < s[i].attr_banks; j++) {
      mstat = CheckAttributeLayout(&s[i].attr[j]);
    }

  }

  return mstat;
}

/**
 * @brief Check for any inconsistencies in the storage layout for attributes
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure to check
 *
 * @return 0 on success, error code otherwise
 */
int CheckAttributeLayout(attr *a) {
  int i = 0, j = 0, mstat = SUCCESS;

    if (a->layout.name == NULL) {
      Message(MESSAGE_ERR, "Attribute name is required\n");
      Error(CORE_ERR_STORAGE);
    }

    /* For scalar attribute reduce the size */
    if (a->layout.dataspace == H5S_SCALAR) {
      a->layout.rank = 1;
      for (j = 0; j < a->layout.rank; j++){
        a->layout.dim[i] = 1;
      }
    }

    if (a->layout.dataspace == H5S_SIMPLE) {
      if (a->layout.rank <= 0) {
        Message(MESSAGE_ERR, "Attribute rank for H5S_SIMPLE dataspace must be > 0\n");
        Error(CORE_ERR_STORAGE);
      }

      if (a->layout.rank > MAX_RANK) {
        Message(MESSAGE_ERR, "Attribute rank for H5S_SIMPLE dataspace must be <= %d\n", MAX_RANK);
        Error(CORE_ERR_STORAGE);
      }

      for (j = 0; j < a->layout.rank; j++) {
        if (a->layout.dim[j] <= 0) {
          Message(MESSAGE_ERR, "Invalid size for attribute dimension %d = %d\n", i, a->layout.dim[j]);
          Error(CORE_ERR_STORAGE);
        }
        a->layout.storage_dim[j] = a->layout.dim[j];
      }
    }

    if ((int)a->layout.datatype <= 0) {
      Message(MESSAGE_ERR, "Attribute datatype is missing\n");
      Error(CORE_ERR_STORAGE);
    }

    for (j = 0; j < MAX_RANK; j++) {
      a->layout.offset[j] = 0; // Offsets are calculated automatically
    }

    a->layout.mpi_datatype = GetMpiDatatype(a->layout.datatype);
    a->layout.datatype_size = H5Tget_size(a->layout.datatype);
    
    a->layout.storage_elements = GetSize(a->layout.rank, a->layout.dim);
    a->layout.elements = GetSize(a->layout.rank, a->layout.dim);

    a->layout.storage_size = (size_t) a->layout.storage_elements * a->layout.datatype_size;
    a->layout.size = (size_t) a->layout.elements * a->layout.datatype_size;
    Message(MESSAGE_DEBUG, "Layout attr '%s': elements %d, storage_elements %d, size = %zu, storage_size = %zu\n",
        a->layout.name, a->layout.storage_elements, a->layout.elements, a->layout.size, a->layout.storage_size);

  return mstat;
}

/**
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
  int mstat = SUCCESS, i = 0;

  for (i = 0; i < banks; i++) {
    mstat = Allocate(&s[i], (size_t)s[i].layout.storage_elements, s[i].layout.datatype_size);
    CheckStatus(mstat);
  
    mstat = CommitAttrMemoryLayout(s[i].attr_banks, s);
    CheckStatus(mstat);
  }

  return mstat;
}

int CommitAttrMemoryLayout(int banks, storage *s) {
  int mstat = SUCCESS, i = 0;

  Message(MESSAGE_DEBUG, "Storage '%s' attr_banks = %d\n", s->layout.name, s->attr_banks);
  for (i = 0; i < banks; i++) {
    Message(MESSAGE_DEBUG, "Attribute '%s', elements = %d, size = %zu\n",
        s->attr[i].layout.name, s->attr[i].layout.storage_elements, s->attr[i].layout.datatype_size);
    mstat = AllocateAttribute(&s->attr[i], (size_t)s->attr[i].layout.storage_elements, s->attr[i].layout.datatype_size);
    CheckStatus(mstat);
  }

  return mstat;
}

/**
 * @brief Frees the memory layout
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure to free
 */
void FreeMemoryLayout(int banks, storage *s) {
  int i = 0, j = 0;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.rank > 0) {
      for (j = 0; j < s[i].attr_banks; j++) {
        FreeAttribute(&s[i].attr[j]);
      }
      if (s[i].memory) {
        Free(&s[i]);
      }
    }
  }
}

/**
 * @brief Commit the storage layout
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int CommitStorageLayout(module *m, pool *p) {
  int mstat = SUCCESS, i = 0, j = 0;
  char path[LRC_CONFIG_LEN];
  hid_t h5location, h5group, h5pools, h5tasks, h5task;

  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  H5CheckStatus(h5location);

  /* The Pools */
  if (!H5Lexists(h5location, POOLS_GROUP, H5P_DEFAULT)) {
    h5pools = H5Gcreate(h5location, POOLS_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(h5pools);
  } else {
    h5pools = H5Gopen(h5location, POOLS_GROUP, H5P_DEFAULT);
    H5CheckStatus(h5pools);
  }

  /* The Pool-ID group */
  sprintf(path, POOL_PATH, p->pid);
  h5group = H5Gcreate(h5pools, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(h5group);

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
  H5CheckStatus(h5tasks);

  /* The task datasets */
  for (i = 0; i < m->task_banks; i++) {
    if (p->task->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[i].layout.storage_type == STORAGE_LIST ||
        p->task->storage[i].layout.storage_type == STORAGE_BOARD) {
          CreateDataset(h5tasks, &p->task->storage[i], m, p);
      }
      if (p->task->storage[i].layout.storage_type == STORAGE_GROUP) {
        for (j = 0; j < p->pool_size; j++) {
          sprintf(path, TASK_PATH, j);
          if (!H5Lexists(h5group, path, H5P_DEFAULT)) {
            h5task = H5Gcreate(h5tasks, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            H5CheckStatus(h5task);
          } else {
            h5task = H5Gopen(h5tasks, path, H5P_DEFAULT);
            H5CheckStatus(h5task);
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
  int mstat = SUCCESS, i;
  query *q;
  setup *o = &(m->layer.setup);
  hid_t h5dataset, h5dataspace;
  hsize_t dims[MAX_RANK];
  herr_t h5status;

  h5dataspace = H5Screate(s->layout.dataspace);
  H5CheckStatus(h5dataspace);

  if (s->layout.dataspace == H5S_SIMPLE) {
    
    for (i = 0; i < MAX_RANK; i++) {
      dims[i] = s->layout.storage_dim[i];
    }

    h5status = H5Sset_extent_simple(h5dataspace, s->layout.rank, dims, NULL);
    H5CheckStatus(h5status);
  }

  h5dataset = H5Dcreate(h5location, s->layout.name, s->layout.datatype, h5dataspace,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(h5dataset);

  q = LoadSym(m, "DatasetPrepare", LOAD_DEFAULT);
  if (q) mstat = q(h5location, h5dataset, p, s, o);
  CheckStatus(mstat);

  H5Dclose(h5dataset);
  H5Sclose(h5dataspace);

  return mstat;
}

/**
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
 * @brief Commit the data to the master file
 *
 * @param h5location The HDF5 location id
 * @param banks Number of memory banks to store
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 */
int CommitData(hid_t h5location, int banks, storage *s) {
  int mstat = SUCCESS, i = 0, j = 0;
  hid_t dataspace, dataset, memspace;
  herr_t hdf_status = 0;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];
  char *buffer = NULL;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf && (int)s[i].layout.size > 0) {

      dataset = H5Dopen(h5location, s[i].layout.name, H5P_DEFAULT);
      H5CheckStatus(dataset);
      dataspace = H5Dget_space(dataset);
      H5CheckStatus(dataspace);

      buffer = calloc(s[i].layout.elements, s[i].layout.datatype_size);
      ReadData(&s[i], buffer);

      /* Whole dataset at once */
      if (s[i].layout.storage_type == STORAGE_GROUP) {

        hdf_status = H5Dwrite(dataset, s[i].layout.datatype,
            H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
        H5CheckStatus(hdf_status);

      } else {
        for (j = 0; j < MAX_RANK; j++) {
          dims[j] = s[i].layout.storage_dim[j];
          offsets[j] = s[i].layout.offset[j];
        }

        memspace = H5Screate_simple(s[i].layout.rank, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offsets, NULL, dims, NULL);
        hdf_status = H5Dwrite(dataset, s[i].layout.datatype,
            memspace, dataspace, H5P_DEFAULT, buffer);
        H5CheckStatus(hdf_status);

        H5Sclose(memspace);
      }

      if (buffer) free(buffer);

      H5Sclose(dataspace);
      H5Dclose(dataset);

    }
  }

  return mstat;
}

/**
 * @brief Commits the attribute to the HDF5 location
 *
 * @param h5location The HDF5 location
 * @param a The attribtue to store
 *
 */
int CommitAttribute(hid_t h5location, attr *a) {
  int mstat = SUCCESS, i = 0;
  hid_t attr_s, attr_d, attr_m;
  herr_t h5status = 0;
  hsize_t dims[MAX_RANK];
  char *buffer = NULL;
      
  buffer = calloc(a->layout.elements, a->layout.datatype_size);
  ReadAttr(a, buffer);

  attr_s = H5Screate(a->layout.dataspace);
  H5CheckStatus(attr_s);

  if (a->layout.dataspace == H5S_SIMPLE) {
    for (i = 0; i < MAX_RANK; i++) {
      dims[i] = a->layout.storage_dim[i];
    }
    
    h5status = H5Sset_extent_simple(attr_s, a->layout.rank, dims, NULL);
    H5CheckStatus(h5status);
  }
  
  attr_d = H5Acreate(h5location, a->layout.name, a->layout.datatype, attr_s, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(attr_d);

  H5Awrite(attr_d, a->layout.datatype, buffer); 

  H5Aclose(attr_d);
  H5Sclose(attr_s);

  if (buffer) free(buffer);

  return mstat;
}

/**
 * @brief Read the dataset data
 *
 * @param h5location The HDF5 location id
 * @param banks Number of memory banks to read
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 */
int ReadDataset(hid_t h5location, int banks, storage *s, int size) {
  int mstat = SUCCESS, i = 0;
  int elements;
  void *buffer = NULL;
  hid_t dataset;
  herr_t hstat;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {
      elements = s[i].layout.elements;
      if (size > 1) {
        elements *= size;
      }
      buffer = calloc(elements, s[i].layout.datatype_size);

      Message(MESSAGE_DEBUG, "Read Data storage name: %s\n", s[i].layout.name);
      dataset = H5Dopen(h5location, s[i].layout.name, H5P_DEFAULT);
      hstat = H5Dread(dataset, s[i].layout.datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
      H5CheckStatus(hstat);
      H5Dclose(dataset);

      WriteData(&s[i], buffer);
      free(buffer);
    }
  }

  return mstat;
}

