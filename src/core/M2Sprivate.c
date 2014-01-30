/**
 * @file
 * Data storage and management
 */
#include "M2Sprivate.h"

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
  unsigned int i = 0, j = 0, task_groups = 0, size = 0;
  size_t len;
  query *q;

  int int_attr;
  long long_attr;
  float float_attr;
  double double_attr;

  /**
   * We need the task board ready as soon as possible
   */

  /* Create the task board programatically */
  p->board->layout.name = "board";
  p->board->layout.rank = TASK_BOARD_RANK + 1;
  p->board->layout.dims[0] = Option2Int("core", "yres", m->layer.setup.head); // vertical res
  p->board->layout.dims[1] = Option2Int("core", "xres", m->layer.setup.head); // horizontal res
  p->board->layout.dims[2] = Option2Int("core", "zres", m->layer.setup.head); // depth res
  p->board->layout.dims[3] = 3; // task status, computing node, task checkpoint number
  p->board->layout.datatype = H5T_NATIVE_SHORT;
  p->board->layout.sync = 1;
  p->board->layout.use_hdf = HDF_NORMAL_STORAGE;
  p->board->layout.storage_type = STORAGE_GROUP;

  /* Programatically create configuration attributes for the task board */
  i = 0;
  p->board->attr_banks = 0;
  while (m->layer.setup.options[i].name[0] != CONFIG_NULL) {
    len = strlen(m->layer.setup.options[i].name);
    p->board->attr[i].layout.name = calloc(len + 1, sizeof(char*));
    if (!p->board->attr[i].layout.name) Error(CORE_ERR_MEM);
        
    strncpy(p->board->attr[i].layout.name, m->layer.setup.options[i].name, len);
    p->board->attr[i].layout.name[len] = CONFIG_NULL;

    p->board->attr[i].layout.dataspace = H5S_SCALAR;
    p->board->attr[i].layout.datatype = GetHDF5Datatype(m->layer.setup.options[i].type);

    p->board->attr_banks++;
    i++;
  }

  /* Check and fix the board layout */
  CheckLayout(m, 1, p->board);
  
  /* Commit task board attributes memory as soon as possible, since we need it during
   * Storage() */
  mstat = CommitAttrMemoryLayout(p->board->attr_banks, p->board);
  CheckStatus(mstat);

  /**
   * Initial runtime configuration 
   * The user can overwrite them in Storage() and PoolPrepare() hooks
   *
   * This is the last time we use the Config linked list
   */
  for (i = 0; i < p->board->attr_banks; i++) {
    if (m->layer.setup.options[i].type == C_STRING) {
      WriteAttr(&p->board->attr[i], m->layer.setup.options[i].value);
    }

    if (m->layer.setup.options[i].type == C_INT || m->layer.setup.options[i].type == C_VAL) {
      int_attr = Option2Int(m->layer.setup.options[i].space, m->layer.setup.options[i].name, m->layer.setup.head);
      WriteAttr(&p->board->attr[i], &int_attr);
    }
    
    if (m->layer.setup.options[i].type == C_LONG) {
      long_attr = Option2Int(m->layer.setup.options[i].space, m->layer.setup.options[i].name, m->layer.setup.head);
      WriteAttr(&p->board->attr[i], &long_attr);
    }
    
    if (m->layer.setup.options[i].type == C_FLOAT) {
      float_attr = Option2Float(m->layer.setup.options[i].space, m->layer.setup.options[i].name, m->layer.setup.head);
      WriteAttr(&p->board->attr[i], &float_attr);
    }

    if (m->layer.setup.options[i].type == C_DOUBLE) {
      double_attr = Option2Double(m->layer.setup.options[i].space, m->layer.setup.options[i].name, m->layer.setup.head);
      WriteAttr(&p->board->attr[i], &double_attr);
    }

    // Special treating for the unit tests
    if (m->test) {
      if (strcmp(m->layer.setup.options[i].name, "restart-mode") == 0) {
        int_attr = 0;
        WriteAttr(&p->board->attr[i], &int_attr);
      }
    }
  }

  /* Update the pool size
   * If the user changes this value, the task board dimensions has to be updated as well */
  p->pool_size = 1;
  for (i = 0; i < TASK_BOARD_RANK; i++) {
    p->pool_size *= p->board->layout.dims[i];
  }
  p->mask_size = p->pool_size;

  /* Load the module storage layout */
  if (m->fallback.handler) {
    q = LoadSym(m, "Storage", LOAD_DEFAULT);
    if (q) mstat = q(p);
    CheckStatus(mstat);
  }

  /* Pool banks */
  p->pool_banks = GetBanks(m->layer.init.banks_per_pool, p->storage);

  for (i = 0; i < p->pool_banks; i++) {
    p->storage[i].attr_banks = 0;
    for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
      if (p->storage[i].attr[j].layout.dataspace == H5S_SIMPLE ||
          p->storage[i].attr[j].layout.dataspace == H5S_SCALAR) {
        p->storage[i].attr_banks++;
      }
    }
  }
  
  /* Right now, there is no support for different storage types of pool datasets */
  for (i = 0; i < p->pool_banks; i++) {
    p->storage[i].layout.storage_type = STORAGE_GROUP;
  }


  /* Compound fields, if any */
  for (i = 0; i < p->pool_banks; i++) {
    p->storage[i].compound_fields = GetFields(m->layer.init.compound_fields, p->storage[i].field);
  }
  
  CheckLayout(m, p->pool_banks, p->storage);
  CommitMemoryLayout(p->pool_banks, p->storage);

  /* The task board size might be overriden by now, check and fix the layout */
  CheckLayout(m, 1, p->board);
  
  /* Commit the task board as late as possible, since we allow to override the size of it
   * during Storage() */
  mstat = Allocate(p->board, p->board->layout.storage_elements, p->board->layout.datatype_size);
  CheckStatus(mstat);

  /* Task Banks */
  p->task_banks = GetBanks(m->layer.init.banks_per_task, p->task->storage);
  for (i = 0; i < p->task_banks; i++) {
    p->task->storage[i].attr_banks = 0;
    for (j = 0; j < m->layer.init.attr_per_dataset; j++) {
      if (p->task->storage[i].attr[j].layout.dataspace == H5S_SIMPLE ||
          p->task->storage[i].attr[j].layout.dataspace == H5S_SCALAR) {
        p->task->storage[i].attr_banks++;
      }
    }
  }

  /* Compound fields, if any */
  for (i = 0; i < p->task_banks; i++) {
    p->task->storage[i].compound_fields = GetFields(m->layer.init.compound_fields, p->task->storage[i].field);
  }
  
  CheckLayout(m, p->task_banks, p->task->storage);

  /* Master only memory/storage operations */
  if (m->node == MASTER) {

    /* Commit memory for task banks (whole datasets) */
    for (i = 0; i < p->task_banks; i++) {
      if (p->task->storage[i].layout.storage_type == STORAGE_GROUP) {
        task_groups = 1;
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);
      }

      if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[i].layout.storage_type == STORAGE_LIST) {

        p->task->storage[i].layout.storage_dim[0] =
          p->task->storage[i].layout.dims[0] * p->pool_size;
        
        for (j = 1; j < MAX_RANK; j++) {
          p->task->storage[i].layout.storage_dim[j] =
            p->task->storage[i].layout.dims[j];
        }
        
        size = GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.storage_dim);
        p->task->storage[i].layout.storage_elements = size;
        p->task->storage[i].layout.storage_size = size * p->task->storage[i].layout.datatype_size;
        mstat = Allocate(&(p->task->storage[i]), size, p->task->storage[i].layout.datatype_size);
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);
      }

      if (p->task->storage[i].layout.storage_type == STORAGE_TEXTURE) {
        p->task->storage[i].layout.storage_dim[0] =
          p->task->storage[i].layout.dims[0] * p->board->layout.dims[0];
        p->task->storage[i].layout.storage_dim[1] =
          p->task->storage[i].layout.dims[1] * p->board->layout.dims[1];
        p->task->storage[i].layout.storage_dim[2] =
          p->task->storage[i].layout.dims[2] * p->board->layout.dims[2];

        for (j = 3; j < MAX_RANK; j++) {
          p->task->storage[i].layout.storage_dim[j] =
            p->task->storage[i].layout.dims[j];
        }
        
        size = GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.storage_dim);
        p->task->storage[i].layout.storage_elements = size;
        p->task->storage[i].layout.storage_size = size * p->task->storage[i].layout.datatype_size;
        mstat = Allocate(&(p->task->storage[i]), size, p->task->storage[i].layout.datatype_size);
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);
      }

      for (j = 0; j < MAX_RANK; j++) {
        p->task->storage[i].layout.offsets[j] = 0;
      }

    }

    /* @todo FIX IT! THIS PART IS TRAGIC FOR MEMORY ALLOCATION IN HUGE RUNS */
    if (task_groups == 1) {
      p->tasks = calloc(p->pool_size, sizeof(task));
      for (i = 0; i < p->pool_size; i++) {
        p->tasks[i] = M2TaskLoad(m, p, i);

        /**
         * @todo Different attributes for each task
         *
         * This probably requires worker nodes to know something on attributes...
         */

      }
    }

    /* Commit the storage layout */
    if (m->mode != RESTART_MODE) {
      CommitStorageLayout(m, p);
    }

  }

  /* Worker operations */
  if (m->node != MASTER) {
    for (i = 0; i < p->task_banks; i++) {
      if (p->task->storage[i].layout.storage_type == STORAGE_TEXTURE ||
          p->task->storage[i].layout.storage_type == STORAGE_LIST ||
          p->task->storage[i].layout.storage_type == STORAGE_PM3D) {

        for (j = 0; j < MAX_RANK; j++) {
          p->task->storage[i].layout.storage_dim[j] =
            p->task->storage[i].layout.dims[j];
        }

        size = GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.storage_dim);
        p->task->storage[i].layout.storage_elements = size;
        p->task->storage[i].layout.storage_size = size * p->task->storage[i].layout.datatype_size;
        mstat = Allocate(&(p->task->storage[i]), size, p->task->storage[i].layout.datatype_size);
        mstat = CommitAttrMemoryLayout(p->task->storage[i].attr_banks, &p->task->storage[i]);

        for (j = 0; j < MAX_RANK; j++) {
          p->task->storage[i].layout.offsets[j] = 0;
        }
      } else {
        task_groups = 1;
      }

      if (task_groups == 1) {
        p->tasks = calloc(1, sizeof(task));
        p->tasks[0] = M2TaskLoad(m, p, 0);
      }
    }
  }
    
  /* Set the checkpoint size at the very end of the storage stage */
  MReadOption(p, "checkpoint", &p->checkpoint_size);

  if (p->checkpoint_size <= 0) {
    p->checkpoint_size = 2048;
    MWriteOption(p, "checkpoint", &p->checkpoint_size);
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
int CheckLayout(module *m, unsigned int banks, storage *s) {
  unsigned int i = 0, j = 0, mstat = SUCCESS;
  size_t datatype_size = 0, storage_size = 0;

  for (i = 0; i < banks; i++) {
    datatype_size = 0; storage_size = 0;
      
    if (s[i].layout.name == NULL) {
      Message(MESSAGE_ERR, "The storage name is required\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.rank <= 1) {
      Message(MESSAGE_ERR, "Rank must be > 1\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.rank > MAX_RANK) {
      Message(MESSAGE_ERR, "Rank must be <= %d\n", MAX_RANK);
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.storage_type == STORAGE_TEXTURE &&
        s[i].layout.rank < TASK_BOARD_RANK) {
      Message(MESSAGE_ERR, "Minimum rank for STORAGE_TEXTURE is TASK_BOARD_RANK (= 3)\n");
      Error(CORE_ERR_STORAGE);
    }

    for (j = 0; j < s[i].layout.rank; j++) {
      if (s[i].layout.dims[j] < 1) {
        Message(MESSAGE_ERR, "Invalid size for dimension %d = %d\n", j, s[i].layout.dims[j]);
        Error(CORE_ERR_STORAGE);
      }
      s[i].layout.storage_dim[j] = s[i].layout.dims[j];
    }

    if (s[i].layout.datatype <= 0) {
      Message(MESSAGE_ERR, "Datatype is missing\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.storage_type < STORAGE_GROUP) {
      Message(MESSAGE_ERR, "The storage type is missing\n");
      Error(CORE_ERR_STORAGE);
    }
    
    if (s[i].layout.storage_type > STORAGE_LIST) {
      Message(MESSAGE_ERR, "Unknown storage type\n");
      Error(CORE_ERR_STORAGE);
    }

    if (s[i].layout.use_hdf) {
      s[i].layout.dataspace = H5S_SIMPLE;
      for (j = 0; j < MAX_RANK; j++) {
        s[i].layout.offsets[j] = 0; // Offsets are calculated automatically
      }

      /* Check for mistakes */
      if (s[i].layout.sync != 1) {
        s[i].layout.sync = 1;
      }
    }

    if (s[i].layout.datatype != H5T_COMPOUND) {
      s[i].layout.mpi_datatype = GetMpiDatatype(s[i].layout.datatype);
      s[i].layout.datatype_size = H5Tget_size(s[i].layout.datatype);
    }
    
    s[i].layout.storage_elements = 
      s[i].layout.elements = GetSize(s[i].layout.rank, s[i].layout.dims);

    /* Calculate storage for H5T_COMPOUND */
    if (s[i].layout.datatype == H5T_COMPOUND) {
      for (j = 0; j < s[i].compound_fields; j++) {
        mstat = CheckFieldLayout(&s[i].field[j]);
        storage_size += s[i].field[j].layout.storage_size;
        datatype_size += s[i].field[j].layout.size;
      }
    } else {
      storage_size = s[i].layout.datatype_size;
      datatype_size = s[i].layout.datatype_size;
    }

    s[i].layout.datatype_size = datatype_size;
    s[i].layout.storage_size = s[i].layout.storage_elements * storage_size;
    s[i].layout.size = s[i].layout.elements * datatype_size;

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
 * @param s The attribute structure to check
 *
 * @return 0 on success, error code otherwise
 */
int CheckAttributeLayout(attr *a) {
  int j = 0, mstat = SUCCESS;

    if (a->layout.name == NULL) {
      Message(MESSAGE_ERR, "Attribute name is required\n");
      Error(CORE_ERR_STORAGE);
    }

    /* For scalar attribute reduce the size */
    if (a->layout.dataspace == H5S_SCALAR) {
      a->layout.rank = 1;
      for (j = 0; j < a->layout.rank; j++){
        a->layout.dims[j] = 1;
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
        if (a->layout.dims[j] <= 0) {
          Message(MESSAGE_ERR, "Invalid size for attribute dimension %d = %d\n", j, a->layout.dims[j]);
          Error(CORE_ERR_STORAGE);
        }
        a->layout.storage_dim[j] = a->layout.dims[j];
      }
    }

    if (a->layout.datatype <= 0) {
      Message(MESSAGE_ERR, "Attribute datatype is missing\n");
      Error(CORE_ERR_STORAGE);
    }

    for (j = 0; j < MAX_RANK; j++) {
      a->layout.offsets[j] = 0; // Offsets are calculated automatically
    }

    a->layout.mpi_datatype = GetMpiDatatype(a->layout.datatype);
    a->layout.datatype_size = H5Tget_size(a->layout.datatype);
    
    if (a->layout.datatype == H5T_NATIVE_CHAR || a->layout.datatype == H5T_C_S1) {
      a->layout.storage_elements = CONFIG_LEN;
      a->layout.elements = CONFIG_LEN;
    } else {
      a->layout.storage_elements = GetSize(a->layout.rank, a->layout.dims);
      a->layout.elements = GetSize(a->layout.rank, a->layout.dims);
    }

    a->layout.storage_size = a->layout.storage_elements * a->layout.datatype_size;
    a->layout.size = a->layout.elements * a->layout.datatype_size;
    Message(MESSAGE_DEBUG, "Layout attr '%s': elements %d, storage_elements %d, size = %zu, storage_size = %zu\n",
        a->layout.name, a->layout.storage_elements, a->layout.elements, a->layout.size, a->layout.storage_size);

  return mstat;
}

/**
 * @brief Check for any inconsistencies in the storage layout for compound fields
 *
 * @param s The field structure to check
 *
 * @return 0 on success, error code otherwise
 */
int CheckFieldLayout(field *field) {
  int mstat = SUCCESS;
  unsigned int j = 0, k = 0;
  size_t padding = 0;

  if (field->layout.name == NULL) {
    Message(MESSAGE_ERR, "Field name is required\n");
    Error(CORE_ERR_STORAGE);
  }

  if (field->layout.datatype <= 0) {
    Message(MESSAGE_ERR, "Field datatype is required\n");
    Error(CORE_ERR_STORAGE);
  }

  if (field->layout.field_offset < 0) {
    Message(MESSAGE_ERR, "Field offset is required\n");
    Error(CORE_ERR_STORAGE);
  }

  if (field->layout.rank < 1) {
    Message(MESSAGE_ERR, "Field rank must be > 1\n");
    Error(CORE_ERR_STORAGE);
  }

  if (field->layout.rank > MAX_RANK) {
    Message(MESSAGE_ERR, "Field rank  must be <= %d\n", MAX_RANK);
    Error(CORE_ERR_STORAGE);
  }

  for (j = 0; j < field->layout.rank; j++) {
    if (field->layout.dims[j] <= 0) {
      Message(MESSAGE_ERR, "Invalid size for field dimension %d = %d\n", j, field->layout.dims[j]);
      Error(CORE_ERR_STORAGE);
    }
    field->layout.storage_dim[j] = field->layout.dims[j];
  }

  field->layout.mpi_datatype = GetMpiDatatype(field->layout.datatype);
  field->layout.datatype_size = H5Tget_size(field->layout.datatype);

  field->layout.elements = 1;
  for (k = 0; k < field->layout.rank; k++) {
    field->layout.elements *= field->layout.dims[k];
  }
  field->layout.storage_elements = field->layout.elements;

  // Calculate padding
  padding = GetPadding(field->layout.elements, field->layout.datatype_size);
  field->layout.storage_size = 
     field->layout.storage_elements * field->layout.datatype_size + padding;
  field->layout.size = 
     field->layout.elements * field->layout.datatype_size + padding;

  return mstat;
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
  int mstat = SUCCESS;
  unsigned int i = 0, j = 0;
  char path[CONFIG_LEN];
  hid_t h5location, h5group, h5pools, h5tasks, h5task;

  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  H5CheckStatus(h5location);

  /* The Pools */
  if (!H5Lexists(h5location, POOLS_GROUP, H5P_DEFAULT)) {
    h5pools = H5Gcreate2(h5location, POOLS_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(h5pools);
  } else {
    h5pools = H5Gopen2(h5location, POOLS_GROUP, H5P_DEFAULT);
    H5CheckStatus(h5pools);
  }

  /* The Pool-ID group */
  sprintf(path, POOL_PATH, p->pid);
  h5group = H5Gcreate2(h5pools, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(h5group);

  /* The Pool-ID task board */
  CreateDataset(h5group, p->board, m, p);

  /* The Pool-ID datasets */
  for (i = 0; i < p->pool_banks; i++) {
    if (p->storage[i].layout.use_hdf) {
      CreateDataset(h5group, &p->storage[i], m, p);
    }
  }

  /* The Tasks group */
  h5tasks = H5Gcreate2(h5group, TASKS_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(h5tasks);

  /* The task datasets */
  for (i = 0; i < p->task_banks; i++) {
    if (p->task->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[i].layout.storage_type == STORAGE_LIST ||
        p->task->storage[i].layout.storage_type == STORAGE_TEXTURE) {
          CreateDataset(h5tasks, &p->task->storage[i], m, p);
      }
      if (p->task->storage[i].layout.storage_type == STORAGE_GROUP) {
        for (j = 0; j < p->pool_size; j++) {
          sprintf(path, TASK_PATH, j);
          if (!H5Lexists(h5tasks, path, H5P_DEFAULT)) {
            h5task = H5Gcreate2(h5tasks, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            H5CheckStatus(h5task);
          } else {
            h5task = H5Gopen2(h5tasks, path, H5P_DEFAULT);
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
  hid_t h5dataset, h5dataspace, h5datatype;
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

  if (s->layout.datatype != H5T_COMPOUND) {
    h5dataset = H5Dcreate2(h5location, s->layout.name, s->layout.datatype, h5dataspace,
        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(h5dataset);
  } else {
    h5datatype = CommitFileDatatype(s);
    h5dataset = H5Dcreate2(h5location, s->layout.name, h5datatype, h5dataspace,
        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(h5dataset);
  }

  q = LoadSym(m, "DatasetPrepare", LOAD_DEFAULT);
  if (q) mstat = q(h5location, h5dataset, p, s);
  CheckStatus(mstat);

  if (s->layout.datatype == H5T_COMPOUND) {
    H5Tclose(h5datatype);
  }
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
unsigned int GetBanks(unsigned int allocated_banks, storage *s) {
  unsigned int banks_in_use = 0;
  unsigned int i = 0;

  for (i = 0; i < allocated_banks; i++) {
    if (s[i].layout.rank > 0) {
      banks_in_use++;
    }
  }

  return banks_in_use;
}

/**
 * @brief Get the number of used compound fields
 *
 * @param allocate_fields Number of allocated compound fields
 * @param field The field structure
 *
 * @return The number of compound fields in use, 0 otherwise
 */
unsigned int GetFields(unsigned int allocated_fields, field *f) {
  unsigned int fields_in_use = 0;
  unsigned int i = 0;

  for (i = 0; i < allocated_fields; i++) {
    if (f[i].layout.rank > 0) {
      fields_in_use++;
    }
  }
  return fields_in_use;
}

