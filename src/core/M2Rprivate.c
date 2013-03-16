/**
 * @file
 * The restart mode
 */
#include "M2Rprivate.h"

/**
 * @brief Performs restart-related tasks
 *
 * @param m The module pointer
 * @param pools The pointer to all pools data
 * @param pool_counter The pool_counter used during recreating the storage layout
 *
 * @return 0 on success, error code otherwise
 */
int Restart(module *m, pool **pools, int *pool_counter) {
  int mstat = SUCCESS;
  int i, j, k, size;
  char path[CONFIG_LEN], task_path[CONFIG_LEN];
  hid_t h5location, group, tasks, task_id, attr_id, hstat;

  if (m->node == MASTER) {
    h5location = H5Fopen(m->filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    H5CheckStatus(h5location);

    /* (A) Get the last pool ID and set the pool counter */
    group = H5Gopen2(h5location, LAST_GROUP, H5P_DEFAULT);
    H5CheckStatus(group);

    attr_id = H5Aopen_name(group, "ID");
    H5CheckStatus(attr_id);

    hstat = H5Aread(attr_id, H5T_NATIVE_UINT, pool_counter);
    H5CheckStatus(hstat);

    H5Aclose(attr_id);
    H5Gclose(group);

    Message(MESSAGE_INFO, "Last pool ID: %d\n", *pool_counter);
  }

  /* (B) Recreate the memory layout */
  MPI_Bcast(pool_counter, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  for (i = 0; i <= *pool_counter; i++) {
    Storage(m, pools[i]);
  }

  /* (C) Read data for all pools */
  if (m->node == MASTER) {
    for (i = 0; i <= *pool_counter; i++) {
      sprintf(path, POOL_PATH, pools[i]->pid);
      group = H5Gopen2(h5location, path, H5P_DEFAULT);
      H5CheckStatus(group);

      /* Read pool reset and stage counters */
      attr_id = H5Aopen_name(group, "RID");
      H5CheckStatus(attr_id);
    
      hstat = H5Aread(attr_id, H5T_NATIVE_UINT, &pools[i]->rid);
      H5CheckStatus(hstat);

      H5Aclose(attr_id);

      attr_id = H5Aopen_name(group, "SID");
      H5CheckStatus(attr_id);
    
      hstat = H5Aread(attr_id, H5T_NATIVE_UINT, &pools[i]->sid);
      H5CheckStatus(hstat);

      H5Aclose(attr_id);
      
      attr_id = H5Aopen_name(group, "SRID");
      H5CheckStatus(attr_id);
    
      hstat = H5Aread(attr_id, H5T_NATIVE_UINT, &pools[i]->srid);
      H5CheckStatus(hstat);

      H5Aclose(attr_id);
      
      Message(MESSAGE_DEBUG, "Pool %d RID: %d SID: %d SRID: %d\n", pools[i]->pid, pools[i]->rid, pools[i]->sid, pools[i]->srid);

      /* Read pool board */
      ReadDataset(group, 1, pools[i]->board, 1);

      /* Read pool storage banks */
      for (j = 0; j < pools[i]->pool_banks; j++) {
        size = GetSize(pools[i]->storage[j].layout.rank, pools[i]->storage[j].layout.dims);
        if (size > 0 && pools[i]->storage[j].layout.use_hdf) {
          ReadDataset(group, 1, &(pools[i]->storage[j]), 1);
        }
      }

      /* Read task data */
      tasks = H5Gopen2(group, "Tasks", H5P_DEFAULT);
      H5CheckStatus(tasks);

      /* Read simple datasets */
      for (j = 0; j < pools[i]->task_banks; j++) {
        size = GetSize(pools[i]->task->storage[j].layout.rank, pools[i]->task->storage[j].layout.dims);
        if (size > 0 && pools[i]->task->storage[j].layout.use_hdf) {
          if (pools[i]->task->storage[j].layout.storage_type == STORAGE_PM3D
            || pools[i]->task->storage[j].layout.storage_type == STORAGE_LIST
            || pools[i]->task->storage[j].layout.storage_type == STORAGE_BOARD) {
            ReadDataset(tasks, 1, &(pools[i]->task->storage[j]), pools[i]->pool_size);
          }
        }
      }

      /* Read datasets inside TaskID groups */
      for (j = 0; j < pools[i]->pool_size; j++) {
        for (k = 0; k < pools[i]->task_banks; k++) {
          if (pools[i]->task->storage[k].layout.use_hdf
              && pools[i]->task->storage[k].layout.storage_type == STORAGE_GROUP) {
            size = GetSize(pools[i]->task->storage[k].layout.rank, pools[i]->task->storage[k].layout.dims);
            if (size > 0) {
              sprintf(task_path, TASK_PATH, k);
              task_id = H5Gopen2(tasks, task_path, H5P_DEFAULT);
              H5CheckStatus(task_id);

              ReadDataset(task_id, 1, &(pools[i]->tasks[j]->storage[k]), 1);
              H5Gclose(task_id);
            }
          }
        }
      }

      H5Gclose(tasks);
      H5Gclose(group);
    }

    H5Fclose(h5location);
  }

  /* Broadcast pool data */
  for (i = 0; i <= *pool_counter; i++) {
    for (j = 0; j < pools[i]->pool_banks; j++) {
      if (pools[i]->storage[j].layout.sync) {
        if (pools[i]->storage[j].layout.elements > 0) {
          MPI_Bcast(&(pools[i]->storage[j].memory[0]), pools[i]->storage[j].layout.elements, 
              pools[i]->storage[j].layout.mpi_datatype, MASTER, MPI_COMM_WORLD);
        }
      }
    }
    // Counters
    MPI_Bcast(&(pools[i]->pid), 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&(pools[i]->rid), 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&(pools[i]->sid), 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&(pools[i]->srid), 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
  }

  return mstat;
}

/**
 * @brief Validate the checkpoint file
 *
 * @param m The module pointer
 * @param filename The name of the file to be validated
 *
 * @return 0 on success, error code otherwise
 */
int Validate(module *m, char *filename) {
  double api;
  char module_name[CONFIG_LEN];
  hid_t h5location, attr, atype;
  herr_t h5status;

  h5location = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  // Get the API
  attr = H5Aopen_name(h5location, "API");
  h5status = H5Aread(attr, H5T_NATIVE_DOUBLE, &api);
  H5CheckStatus(h5status);
  H5Aclose(attr);

  // Get the module
  attr = H5Aopen_name(h5location, "MODULE");
  atype  = H5Aget_type(attr);
  h5status = H5Aread(attr, atype, &module_name);
  H5CheckStatus(h5status);
  H5Aclose(attr);

  H5Fclose(h5location);

  Message(MESSAGE_INFO, "Restart mode validation: API = %.2f, MODULE = %s\n", api, module_name);

  // Validate the module
  if (strcmp(module_name, Option2String("core", "module", m->layer.setup.head)) != 0) {
    Message(MESSAGE_ERR, "Module mismatch. Expected '%s', got '%s'\n", module_name, Option2String("core", "module", m->layer.setup.head)); 
    return -CORE_ERR_RESTART;
  }

  // Validate the API
  if (api < PACKAGE_VERSION_API) {
    Message(MESSAGE_ERR, "API mismatch. Expected %.2f, got %.2f\n", PACKAGE_VERSION_API, api);
    return -CORE_ERR_RESTART;
  }

  return SUCCESS;
}

