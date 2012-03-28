/**
 * @file
 * The storage stage
 */
#include "MStorage.h"

/**
 * @function
 */
int Storage(module *m, pool *p) {
  int mstat = 0;
  int i, j;
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

  m->pool_banks = GetBanks(m->layer.init.banks_per_pool, p->storage);
  m->task_banks = GetBanks(m->layer.init.banks_per_task, p->task->storage);
  p->pool_size = GetSize(p->board->layout.rank, p->board->layout.dim);

  /* Commit memory layout used during the task pool */
  CommitMemoryLayout(m->pool_banks, p->storage);

  /* Commit the storage layout (only the Master node) */
  if (m->node == MASTER) {
    CheckLayout(m->pool_banks, p->storage);
    CommitStorageLayout(m, p);
  }

  /* Commit Board */
  p->board->data = AllocateBuffer(p->board->layout.rank,p->board->layout.dim);
  for (i = 0; i < p->board->layout.dim[0]; i++) {
    for (j = 0; j < p->board->layout.dim[j]; j++) {
      p->board->data[i][j] = TASK_AVAILABLE;
    }
  }

  return mstat;
}

/**
 * @function
 * Checks inconsistencies in the storage layout
 */
int CheckLayout(int banks, storage *s) {
  int i = 0, mstat = 0;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {
      /* Common fixes before future development */
      s[i].layout.rank = 2;
      s[i].layout.dataspace_type = H5S_SIMPLE;
      s[i].layout.datatype = H5T_NATIVE_DOUBLE;

      /* Check for mistakes */
      if (s[i].layout.path == NULL) {
        Message(MESSAGE_ERR, "The storage path is required when use_hdf\n");
        Error(CORE_ERR_STORAGE);
      }
      /*if (s[i].layout.rank == 0) {
        Message(MESSAGE_ERR, "Rank must be > 0 when use_hdf\n");
        Error(CORE_ERR_STORAGE);
      }*/
    }
  }
  return mstat;
}

/**
 * @function
 * Commits the memory layout
 *
 * This function must run on every node -- the size of data arrays shared between nodes
 * depends on the memory layout.
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
 * Frees the memory
 */
void FreeMemoryLayout(int banks, storage *s) {
  int i = 0;

  for (i = 0; i < banks; i++) {
    if (s[i].data) {
      FreeBuffer(s[i].data, s[i].layout.dim);
    }
  }
}

/**
 * @function
 * Commit the storage layout
 */
int CommitStorageLayout(module *m, pool *p) {
  int mstat = 0, i = 0, j = 0;
  hid_t h5location, h5group, h5pools, h5tasks, h5task;
  char path[LRC_CONFIG_LEN];

  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);

  /* The Pools */
  if (!H5Lexists(h5location, "/Pools", H5P_DEFAULT)) {
    h5pools = H5Gcreate(h5location, "/Pools", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  } else {
    h5pools = H5Gopen(h5location, "/Pools", H5P_DEFAULT);
  }
    
  /* The Pool-ID group */
  sprintf(path, "/Pools/pool-%04d", p->pid);
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
  h5tasks = H5Gcreate(h5group, "Tasks", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* The task datasets */
  for (i = 0; i < m->task_banks; i++) {
    if (p->task->storage[i].layout.storage_type == STORAGE_PM3D ||
      p->task->storage[i].layout.storage_type == STORAGE_BOARD) {
        CreateDataset(h5tasks, &p->task->storage[i], m, p);
    }
    if (p->task->storage[i].layout.storage_type == STORAGE_BASIC) {
      for (j = 0; j < p->pool_size; j++) {
        sprintf(path, "task-%04d", j);
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

  H5Gclose(h5tasks);
  H5Gclose(h5group);
  H5Gclose(h5pools);
  H5Fclose(h5location);
  return mstat;
}

/**
 * @function
 * Creates dataset
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
    if (s->layout.storage_type == STORAGE_PM3D) {
      dims[0] = s->layout.dim[0] * p->pool_size;
      dims[1] = s->layout.dim[1];
    }
    if (s->layout.storage_type == STORAGE_BOARD) {
      dims[0] = s->layout.dim[0] * p->board->layout.dim[0];
      dims[1] = s->layout.dim[1] * p->board->layout.dim[1];
    }
    h5status = H5Sset_extent_simple(h5dataspace, s->layout.rank, dims, NULL);
  }

  h5dataset = H5Dcreate(h5location, s->layout.path, s->layout.datatype, h5dataspace,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  H5Dclose(h5dataset);
  H5Sclose(h5dataspace);

  return mstat;
}

/**
 * @function
 * Gets the number of used memory banks.
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
 * Commits data to the master file
 */
int CommitData(hid_t h5location, int banks, storage *s, int flag, hsize_t *board_dims, hsize_t *offsets) {
  int mstat = 0, i = 0;
  hid_t dataspace, dataset, memspace;
  herr_t hdf_status;
  hsize_t ldims[MAX_RANK];

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
        ldims[0] = s[i].layout.dim[0];
        ldims[1] = s[i].layout.dim[1];
        memspace = H5Screate_simple(s[i].layout.rank, ldims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offsets, NULL, ldims, NULL);
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

