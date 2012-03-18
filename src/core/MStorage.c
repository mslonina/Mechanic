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

  /* Commit memory layout used during the task pool */
  CommitMemoryLayout(p->storage);
  
  /* Commit the storage layout (only the Master node) */
  if (m->node == MASTER) {
    CommitStorageLayout(p->location, p->storage);
  }

  m->pool_banks = GetBanks(p->storage);
  m->task_banks = GetBanks(p->task.storage);

//  printf("BANKS :: pool %d, task %d\n", m->pool_banks, m->task_banks);

  return mstat;
}

/** 
 * Commits the storage layout to the HDF5 datafile
 */ 
int CommitStorageLayout(hid_t location, storage *s) {
  int mstat = 0, i = 0;
  hid_t dataspace, dataset;
  herr_t hdf_status;
  hsize_t dims[MAX_RANK];

  while (s[i].layout.path) {
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
    i++;
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
int CommitMemoryLayout(storage *s) {
  int mstat = 0, i = 0;

  while (s[i].layout.path) {
    s[i].data = AllocateDoubleArray(s[i].layout.rank, s[i].layout.dim);
    i++;
  }

  return mstat;
}

/**
 * @function
 * Frees the memory
 */
void FreeMemoryLayout(storage *s) {
  int i = 0;

  while (s[i].layout.path) {
    if (s[i].data) {
      FreeDoubleArray(s[i].data, s[i].layout.dim);
    }
    i++;
  }
}

/**
 * @function
 * Gets the number of used memory banks.
 */
int GetBanks(storage *s) {
  int banks = 0;
  while (s[banks].layout.path) {
    banks++;
  }
  return banks;
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
