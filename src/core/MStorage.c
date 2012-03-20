/**
 * @file
 * The storage stage.
 */
#include "MStorage.h"

/**
 * @function
 */
int Storage(module *m, pool *p) {
  int mstat = 0;
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

  /* Commit memory layout used during the task pool */
  CommitMemoryLayout(m->pool_banks, p->storage);
  
  /* Commit the storage layout (only the Master node) */
  if (m->node == MASTER) {
    CheckLayout(m->pool_banks, p->storage);
    CommitStorageLayout(p->location, m->pool_banks, p->storage);
  }

//  printf("BANKS :: pool %d, task %d\n", m->pool_banks, m->task_banks);

  return mstat;
}

/**
 * @function
 * Checks inconsitencies in the storage layout
 */
int CheckLayout(int banks, storage *s) {
  int i = 0, mstat = 0;
  
  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {
      if (s[i].layout.path == NULL) {
        Message(MESSAGE_ERR, "The storage path is required when use_hdf\n");
        Error(CORE_ERR_STORAGE);
      }
      if (s[i].layout.rank == 0) {
        Message(MESSAGE_ERR, "Rank must be > 0 when use_hdf\n");
        Error(CORE_ERR_STORAGE);
      }
    }
  }
  return mstat;
}

/** 
 * @function
 * Commits the storage layout to the HDF5 datafile
 *
 * @todo
 * The CheckAndFixLayout must run before.
 */ 
int CommitStorageLayout(hid_t location, int banks, storage *s) {
  int mstat = 0, i = 0;
  hid_t dataspace, dataset;
  herr_t hdf_status;
  hsize_t dims[MAX_RANK];

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {
      dataspace = H5Screate(s[i].layout.dataspace_type);
      if (s[i].layout.dataspace_type == H5S_SIMPLE) {
        dims[0] = s[i].layout.dim[0];
        dims[1] = s[i].layout.dim[1];
        hdf_status = H5Sset_extent_simple(dataspace, s[i].layout.rank, dims, NULL);
      }
      dataset = H5Dcreate(location, s[i].layout.path, s[i].layout.datatype, dataspace, 
          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

      H5Dclose(dataset);
      H5Sclose(dataspace);
    }
  }

  if (hdf_status < 0) mstat = CORE_ERR_HDF;
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
    s[i].data = AllocateDoubleArray(s[i].layout.rank, s[i].layout.dim);
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
      FreeDoubleArray(s[i].data, s[i].layout.dim);
    }
  }
}

/**
 * @function
 * Gets the number of used memory banks.
 *
 * @todo
 * This function should return the number of memory banks in use, however, do we really
 * need such thing? Maybe only use CheckAndFixLayout() function to detect inconsitencies
 * when i.e. use_hdf = 1 and path is NULL?
 */
int GetBanks(int allocated_banks, storage *s) {
  int banks_in_use = 0;
  int i = 0;

  for (i = 0; i < allocated_banks; i++) {
    //if (s[i].layout.path != NULL) {
      banks_in_use++;
    //  printf("rank %d, bank: %s\n", s[i].layout.rank, s[i].layout.path);
    //}
  }

  return banks_in_use;
}

/**
 * @function
 * Writes the data buffer to the dataset
 */
int CommitData(hid_t location, storage *s, double **data) {
  int mstat = 0;
  herr_t hdf_status;

  hdf_status = H5Dwrite(location, s->layout.datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0][0]);
  if (hdf_status < 0) mstat = CORE_ERR_HDF;
  return mstat;
}

/**
 * @function
 * Writes all specified data buffers to the master file
 */
int WritePoolData(pool *p) {
  int mstat = 0, i = 0;
  hid_t dataset;
  herr_t hdf_status;
  
  while (p->storage[i].layout.path) {
    if (p->storage[i].layout.use_hdf) {
      dataset = H5Dopen(p->location, p->storage[i].layout.path, H5P_DEFAULT);
      hdf_status = CommitData(dataset, &p->storage[i], p->storage[i].data);
      H5Dclose(dataset);
    }
    i++;
  }
  
  if (hdf_status < 0) mstat = CORE_ERR_HDF;
  return mstat;
}

/**
 * @function
 */
storage* StorageLoad(int banks) {
  storage *s = NULL;

  s = (storage*) malloc(banks * sizeof(storage));
  if (!s) Error(CORE_ERR_MEM);

  return s;
}

/**
 * @function
 */
void StorageFinalize(int banks, storage *s) {

}
