/**
 * @file
 * The storage stage
 */
#include "MStorage.h"

/**
 * @function
 */
/*storage* StorageLoad(module *m, pool*p) {
  storage *s = NULL;
  query *q;

  s = calloc(banks * sizeof(storage), sizeof(storage));
  if (!s) Error(CORE_ERR_MEM);
*/
  /* First load the fallback (core) storage layout */
/*  if (m->fallback.handler) {
    q = LoadSym(m, "Storage", FALLBACK_ONLY);
    if (q) mstat = q(p, &m->layer.setup);
    CheckStatus(mstat);
  }
*/
  /* Load the module setup */
/*  q = LoadSym(m, "Storage", NO_FALLBACK);
  if (q) mstat = q(p, &m->layer.setup);
  CheckStatus(mstat);

  return s;
}*/

/**
 * @function
 */
void StorageFinalize(int banks, storage *s) {

}
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
  //  CommitStorageLayout(p->h5location, m->pool_banks, p->storage);
  //  CommitStorageLayout(p->h5location, 1, p->board); // The task board dataset
  }

  /* Commit Board */
  p->board->data = AllocateBuffer(p->board->layout.rank,p->board->layout.dim);

//  printf("BANKS :: pool %d, task %d\n", m->pool_banks, m->task_banks);

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
 * Commits the storage layout to the HDF5 datafile
 *
 * @todo
 * The CheckAndFixLayout must run before.
 */
int CommitStorageLayout(hid_t h5location, int banks, storage *s) {
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
      dataset = H5Dcreate(h5location, s[i].layout.path, s[i].layout.datatype, dataspace, 
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
    //if (s[i].layout.rank > 0) {
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
int CommitDataset(hid_t h5location, storage *s, double **data) {
  int mstat = 0;
  herr_t hdf_status;

  hdf_status = H5Dwrite(h5location, s->layout.datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0][0]);
  if (hdf_status < 0) mstat = CORE_ERR_HDF;
  return mstat;
}

/**
 * @function
 * Commits data to the master file
 */
int CommitData(hid_t h5location, int banks, storage *s, int flag, hsize_t *board_dims, hsize_t *offsets) {
  int mstat = 0, i = 0;
  hid_t dataspace, dataset, memspace;
  herr_t hdf_status;
  hsize_t dims[MAX_RANK], ldims[MAX_RANK];

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {

      dataspace = H5Screate(s[i].layout.dataspace_type);
      
      if (s[i].layout.dataspace_type == H5S_SIMPLE) {
      
        if (s[i].layout.storage_type == STORAGE_BASIC) {
          dims[0] = s[i].layout.dim[0];
          dims[1] = s[i].layout.dim[1];
        }
        
        if (s[i].layout.storage_type == STORAGE_PM3D) {
          dims[0] = s[i].layout.dim[0] * board_dims[0] * board_dims[1];
          dims[1] = s[i].layout.dim[1];
        }
        
        if (s[i].layout.storage_type == STORAGE_BOARD) {
          dims[0] = s[i].layout.dim[0] * board_dims[0];
          dims[1] = s[i].layout.dim[1] * board_dims[1];
        }
        
        hdf_status = H5Sset_extent_simple(dataspace, s[i].layout.rank, dims, NULL);
      }

      if (!H5Lexists(h5location, s[i].layout.path, H5P_DEFAULT)) {
        dataset = H5Dcreate(h5location, s[i].layout.path, s[i].layout.datatype, dataspace,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

      } else {
        dataset = H5Dopen(h5location, s[i].layout.path, H5P_DEFAULT);
      }
        
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

