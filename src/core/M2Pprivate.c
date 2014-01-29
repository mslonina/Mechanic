/**
 * @file
 * The task pool
 */
#include "M2Pprivate.h"

/**
 * @brief Load the task pool
 *
 * @param m The module pointer
 * @param pid The pool ID
 *
 * @return The pool pointer, NULL otherwise
 */
pool* PoolLoad(module *m, unsigned int pid) {
  unsigned int i = 0, j = 0;
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

    /*p->storage[i].layout.fields = calloc(m->layer.init.compound_fields, sizeof(fields_type));
    if (!p->storage[i].layout.fields) Error(CORE_ERR_MEM);

    for (j = 0; j < m->layer.init.compound_fields; j++) {
      p->storage[i].layout.fields[j] = (fields_type) COMPOUND_TEST;
    }

    for (j = 0; j < m->layer.init.compound_fields; j++) {
      printf("field.name = %s, .rank = %d, .dims = (%d, %d, %d, %d), datatype = %d\n",
          p->storage[i].layout.fields[j].name,
          p->storage[i].layout.fields[j].rank,
          p->storage[i].layout.fields[j].dims[0],
          p->storage[i].layout.fields[j].dims[1],
          p->storage[i].layout.fields[j].dims[2],
          p->storage[i].layout.fields[j].dims[3],
          p->storage[i].layout.fields[j].datatype
          );
    }*/
  }

  /* Allocate task board pointer */
  p->board = calloc(1, sizeof(storage));
  if (!p->board) Error(CORE_ERR_MEM);

  p->board->layout = (schema) STORAGE_END;
  p->board->memory = NULL;

  /* Task board attributes */
  p->board->attr = calloc(m->layer.init.options, sizeof(storage));
  if (!p->board->attr) Error(CORE_ERR_MEM);
  for (j = 0; j < m->layer.init.options; j++) {
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

    /*p->task->storage[i].layout.fields = calloc(m->layer.init.compound_fields, sizeof(fields_type));
    if (!p->task->storage[i].layout.fields) Error(CORE_ERR_MEM);
    for (j = 0; j < m->layer.init.compound_fields; j++) {
      p->task->storage[i].layout.fields[j] = (fields_type) COMPOUND_END;
    }*/
  }

  p->tasks = NULL;

  p->pid = pid;
  p->rid = 0;
  p->sid = 0;
  p->srid = 0;
  p->node = m->node;
  p->mpi_size = m->mpi_size;
  p->completed = 0;

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
  int mstat = SUCCESS;
  query *q;
  setup *s = &(m->layer.setup);

  task *t;
  short ****board_buffer = NULL;
  clock_t time_in, time_out;
  double cpu_time;
  int reversed = 0;
  int reset_checkpoints = 0;
  unsigned int i = 0, j = 0;
  unsigned int x = 0, y = 0, z = 0, k = 0;

  if (m->node == MASTER) {

    p->mask_size = p->pool_size;
    p->completed = 0;

    q = LoadSym(m, "PoolPrepare", LOAD_DEFAULT);
    if (q) mstat = q(all, p);
    CheckStatus(mstat);

    p->state = POOL_PREPARED;
    if (p->mask_size > p->pool_size) p->mask_size = p->pool_size;

    mstat = PoolProcessData(m, p, s);
    CheckStatus(mstat);

    // Prepare the task board
    t = M2TaskLoad(m, p, 0);
    board_buffer = AllocateShort4(p->board);

    // Initialize the task board
    for (x = 0; x < p->board->layout.dims[0]; x++) {
      for (y = 0; y < p->board->layout.dims[1]; y++) {
        for (z = 0; z < p->board->layout.dims[2]; z++) {
          for (k = 0; k < p->board->layout.dims[3]; k++) {
            board_buffer[x][y][z][k] = 0;
          }
        }
      }
    }

    if (p->mask_size != p->pool_size) reversed = 1;

    if (m->mode == RESTART_MODE) {
      ReadData(p->board, &board_buffer[0][0][0][0]);
    }

    MReadOption(p, "reset-checkpoints", &reset_checkpoints);
    
    // Initialize the task board
    for (x = 0; x < p->board->layout.dims[0]; x++) {
      for (y = 0; y < p->board->layout.dims[1]; y++) {
        for (z = 0; z < p->board->layout.dims[2]; z++) {

          // restart mode
          if (m->mode == RESTART_MODE) {
            if (board_buffer[x][y][z][0] == TASK_IN_USE) {
              board_buffer[x][y][z][0] = TASK_TO_BE_RESTARTED;
              if (reset_checkpoints == 1) {
                board_buffer[x][y][z][2] = 0;
              }
            }
            if (reversed) {
              if (board_buffer[x][y][z][0] == TASK_AVAILABLE) {
                board_buffer[x][y][z][0] = TASK_FINISHED;
              }
            }
          } else {
            if (reversed) {
              board_buffer[x][y][z][0] = TASK_FINISHED;
              board_buffer[x][y][z][1] = 0; // reset computing node
              board_buffer[x][y][z][2] = 0; // reset task checkpoint id
            } else {
              board_buffer[x][y][z][0] = TASK_AVAILABLE;
              board_buffer[x][y][z][1] = 0; // reset computing node
              board_buffer[x][y][z][2] = 0; // reset task checkpoint id
            }
          }

          // kept here since it covers also the restart mode
          if (board_buffer[x][y][z][0] == TASK_FINISHED) {
            p->completed++;
          }
        }
      }
    }

    if (m->mode == RESTART_MODE) {
      Message(MESSAGE_INFO, "Completed %d tasks\n", p->completed);
    }

    time_in = clock();
    
    for (i = 0; i < p->mask_size; i++) {
      t->tid = i;

      // do we have to load task data here? (CPU overhead), the pool datasets are
      // available though

      q = LoadSym(m, "TaskBoardMap", LOAD_DEFAULT);
      if (q) mstat = q(p, t);
      CheckStatus(mstat);
      
      q = LoadSym(m, "BoardPrepare", LOAD_DEFAULT);
      if (q) t->state = q(all, p, t);

      if (m->mode != RESTART_MODE) {
        if (t->state == TASK_ENABLED) {
          board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_AVAILABLE;
          if (reversed) p->completed--;
        }

        if (t->state == TASK_DISABLED) {
          board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_FINISHED;
          p->completed++;
        }
      } 
      
      // the logic below somehow works...
      if (m->mode == RESTART_MODE) {
        // skip already finished tasks
        if (board_buffer[t->location[0]][t->location[1]][t->location[2]][0] == TASK_FINISHED) continue;
        
        if (t->state == TASK_ENABLED) {
          board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_AVAILABLE;
          if (reversed) p->completed--;
        }
        
        if (board_buffer[t->location[0]][t->location[1]][t->location[2]][0] == TASK_AVAILABLE) {
          if (t->state == TASK_DISABLED) {
            board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_FINISHED;
            p->completed++;
          }
        }
      }
    }

    time_out = clock();
    cpu_time = (double)(time_out - time_in)/CLOCKS_PER_SEC;
    if (m->showtime) Message(MESSAGE_INFO, "BoardPrepare completed. CPU time: %f\n", cpu_time);

    WriteData(p->board, &board_buffer[0][0][0][0]);

    TaskFinalize(m, p, t);
    free(board_buffer);

  }

  /* Broadcast pool data */
  for (i = 0; i < p->pool_banks; i++) {
    if (p->storage[i].layout.sync) {
      if (p->storage[i].layout.elements > 0) {
        MPI_Bcast(&(p->storage[i].memory[0]), p->storage[i].layout.storage_size,
            MPI_CHAR, MASTER, MPI_COMM_WORLD);
        // Broadcast pool attributes
        for (j = 0; j < p->storage[i].attr_banks; j++) {
          MPI_Bcast(&(p->storage[i].attr[j].memory[0]), p->storage[i].attr[j].layout.storage_size,
              MPI_CHAR, MASTER, MPI_COMM_WORLD);
        }
      }
    }
  }

  /* Broadcast pool setup */
  for (i = 0; i < p->board->attr_banks; i++) {
    MPI_Bcast(&(p->board->attr[i].memory[0]), p->board->attr[i].layout.elements,
        p->board->attr[i].layout.mpi_datatype, MASTER, MPI_COMM_WORLD);
  }

  /* Broadcast task banks attributes */
  for (i = 0; i < p->task_banks; i++) {
    if (p->task->storage[i].layout.storage_type != STORAGE_GROUP) {
      for (j = 0; j < p->task->storage[i].attr_banks; j++) {
        MPI_Bcast(&(p->task->storage[i].attr[j].memory[0]), p->task->storage[i].attr[j].layout.elements,
            p->task->storage[i].attr[j].layout.mpi_datatype, MASTER, MPI_COMM_WORLD);
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
    if (q) pool_create = q(all, p);

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
  unsigned int i = 0, j = 0, k = 0, task_groups = 0;
  char path[CONFIG_LEN];
  query *q;
  hid_t h5location, h5pool, h5tasks, h5task, h5dataset;
  hid_t attr_s, attr_d;
  hid_t hstat;

  /* Do some data processing */
  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  H5CheckStatus(h5location);

  sprintf(path, POOL_PATH, p->pid);
  h5pool = H5Gopen2(h5location, path, H5P_DEFAULT);
  H5CheckStatus(h5pool);

  /* Process task board attributes */
  h5dataset = H5Dopen2(h5pool, p->board->layout.name, H5P_DEFAULT);
  H5CheckStatus(h5dataset);

  for (j = 0; j < p->board->attr_banks; j++) {
    mstat = CommitAttribute(h5dataset, &p->board->attr[j]);
    CheckStatus(mstat);
  }

  H5Dclose(h5dataset);
  
  /* Write pool data */
  mstat = CommitData(h5pool, p->pool_banks, p->storage);
  CheckStatus(mstat);

  /* Write global attributes */
  if (H5Aexists(h5pool, "Status") > 0) {
    attr_d = H5Aopen(h5pool, "Status", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_USHORT, &p->state);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate2(h5pool, "Status", H5T_NATIVE_USHORT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_USHORT, &p->state); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  if (H5Aexists(h5pool, "ID") > 0) {
    attr_d = H5Aopen(h5pool, "ID", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->pid);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate2(h5pool, "ID", H5T_NATIVE_UINT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->pid); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  if (H5Aexists(h5pool, "RID") > 0) {
    attr_d = H5Aopen(h5pool, "RID", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->rid);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate2(h5pool, "RID", H5T_NATIVE_UINT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->rid); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  if (H5Aexists(h5pool, "SID") > 0) {
    attr_d = H5Aopen(h5pool, "SID", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->sid);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate2(h5pool, "SID", H5T_NATIVE_UINT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->sid); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  if (H5Aexists(h5pool, "SRID") > 0) {
    attr_d = H5Aopen(h5pool, "SRID", H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->srid);
  } else {
    attr_s = H5Screate(H5S_SCALAR);
    attr_d = H5Acreate2(h5pool, "SRID", H5T_NATIVE_UINT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_d, H5T_NATIVE_UINT, &p->srid); 
    H5Sclose(attr_s);
  }
      
  H5Aclose(attr_d);

  /* The last pool link */
  if (p->state == POOL_PREPARED) {
    Message(MESSAGE_DEBUG, "Last group: %s\n", path);
    if (H5Lexists(h5location, LAST_GROUP, H5P_DEFAULT)) {
      H5Ldelete(h5location, LAST_GROUP, H5P_DEFAULT);
    }

    hstat = H5Lcreate_hard(h5pool, path, h5location, LAST_GROUP, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(hstat);
  }

  /* Process all datasets in the current pool */
  for (i = 0; i < p->pool_banks; i++) {
    if (p->storage[i].layout.use_hdf) {
      if (p->storage[i].layout.use_hdf == HDF_NORMAL_STORAGE) {
        h5dataset = H5Dopen2(h5pool, p->storage[i].layout.name, H5P_DEFAULT);
        H5CheckStatus(h5dataset);

        q = LoadSym(m, "DatasetProcess", LOAD_DEFAULT);
        if (q) mstat = q(h5pool, h5dataset, p, &(p->storage[i]));
        CheckStatus(mstat);

        for (j = 0; j < p->storage[i].attr_banks; j++) {
          mstat = CommitAttribute(h5dataset, &p->storage[i].attr[j]);
          CheckStatus(mstat);
        }

        H5Dclose(h5dataset);
      }

      // Remove the temporary dataset
      if (p->storage[i].layout.use_hdf == HDF_TEMP_STORAGE) {
        if (p->state == POOL_PROCESSED) {
          H5Ldelete(h5pool, p->storage[i].layout.name, H5P_DEFAULT);
        }
      }
    }
  }

  h5tasks = H5Gopen2(h5pool, TASKS_GROUP, H5P_DEFAULT);

  /* Process all task datasets in the current pool */
  for (i = 0; i < p->task_banks; i++) {
    if (p->task->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.storage_type != STORAGE_GROUP) {
        if (p->task->storage[i].layout.use_hdf == HDF_NORMAL_STORAGE) {
          h5dataset = H5Dopen2(h5tasks, p->task->storage[i].layout.name, H5P_DEFAULT);
          H5CheckStatus(h5dataset);

          q = LoadSym(m, "DatasetProcess", LOAD_DEFAULT);
          if (q) mstat = q(h5tasks, h5dataset, p, &(p->task->storage[i]));
          CheckStatus(mstat);

          for (j = 0; j < p->task->storage[i].attr_banks; j++) {
            mstat = CommitAttribute(h5dataset, &p->task->storage[i].attr[j]);
            CheckStatus(mstat);
          }

          H5Dclose(h5dataset);
        }

        // Remove the temporary dataset
        if (p->task->storage[i].layout.use_hdf == HDF_TEMP_STORAGE) {
          if (p->state == POOL_PROCESSED) {
            H5Ldelete(h5tasks, p->task->storage[i].layout.name, H5P_DEFAULT);
          }
        }
      } else {
        task_groups = 1;
      }
    }
  }

  /* This may be memory heavy */
  if (task_groups == 1) {
    for (i = 0; i < p->pool_size; i++) {
      sprintf(path, TASK_PATH, i);
      h5task = H5Gopen2(h5tasks, path, H5P_DEFAULT);

      for (j = 0; j < p->task_banks; j++) {
        if (p->task->storage[j].layout.use_hdf &&
            p->task->storage[j].layout.storage_type == STORAGE_GROUP) {

          if (p->task->storage[j].layout.use_hdf == HDF_NORMAL_STORAGE) {

            h5dataset = H5Dopen2(h5task, p->task->storage[j].layout.name, H5P_DEFAULT);
            H5CheckStatus(h5dataset);
          
            q = LoadSym(m, "DatasetProcess", LOAD_DEFAULT);
            if (q) mstat = q(h5task, h5dataset, p, &(p->task->storage[j]));
            CheckStatus(mstat);

            for (k = 0; k < p->task->storage[j].attr_banks; k++) {
              mstat = CommitAttribute(h5dataset, &p->task->storage[j].attr[k]);
              CheckStatus(mstat);
            }

            H5Dclose(h5dataset);
          }

          // Remove the temporary dataset
          if (p->task->storage[i].layout.use_hdf == HDF_TEMP_STORAGE) {
            if (p->state == POOL_PROCESSED) {
              H5Ldelete(h5task, p->task->storage[i].layout.name, H5P_DEFAULT);
            }
          }
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
  unsigned int i, j, k, l;
  hid_t h5location, group;
  char path[CONFIG_LEN];
  short ****board;

  /* Reset the board memory banks */
  if (m->node == MASTER) {
    p->completed = 0;
    board = AllocateShort4(p->board);

    // Memset
    for (i = 0; i < p->board->layout.dims[0]; i++) {
      for (j = 0; j < p->board->layout.dims[1]; j++) {
        for (k = 0; k < p->board->layout.dims[2]; k++) {
          for (l = 0; l < p->board->layout.dims[3]; l++) {
            board[i][j][k][l] = TASK_AVAILABLE;
          }
        }
      }
    }
    
    WriteData(p->board, &board[0][0][0][0]);

    /* Reset the board storage banks */
    h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
    H5CheckStatus(h5location);

    sprintf(path, POOL_PATH, p->pid);
    group = H5Gopen2(h5location, path, H5P_DEFAULT);
    H5CheckStatus(group);

    mstat = CommitData(group, 1, p->board);
    CheckStatus(mstat);

    H5Gclose(group);
    H5Fclose(h5location);

    free(board);
  }

  // Reset stages
  p->sid = 0;
  p->srid = 0;

  return mstat;
}

/**
 * @brief Finalize the pool
 *
 * @param m The module pointer
 * @param p The pool pointer to finalize
 */
void PoolFinalize(module *m, pool *p) {
  unsigned int i = 0;

  if (p) {
    if (p->storage) {
      for (i = 0; i < p->pool_banks; i++) {
        free(p->storage[i].attr);
//        free(p->storage[i].layout.fields);
      }
      FreeMemoryLayout(p->pool_banks, p->storage);
      free(p->storage);
    }

    if (p->task) {
      if (p->task->storage) {
        for (i = 0; i < p->task_banks; i++) {
          free(p->task->storage[i].attr);
//          free(p->task->storage[i].layout.fields);
        }
        FreeMemoryLayout(p->task_banks, p->task->storage);
        free(p->task->storage);
      }

      free(p->task);
    }

    if (p->tasks) {
      if (m->node == MASTER) {
        for (i = 0; i < p->pool_size; i++) {
          TaskFinalize(m, p, p->tasks[i]);
        }
      } else {
        TaskFinalize(m, p, p->tasks[0]);
      }

      free(p->tasks);
    }

    if (p->board) {
      for (i = 0; i < p->board->attr_banks; i++) {
        free(p->board->attr[i].layout.name);
      }
      FreeMemoryLayout(1, p->board);
      free(p->board->attr);
      free(p->board);
    }

    free(p);
  }
}

