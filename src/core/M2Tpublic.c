/**
 * An essential aspect of creativity is not being afraid to fail
 *
 * Edwin Land
 */

/**
 * @file
 * The task (public API)
 */
#include "M2Tpublic.h"

/**
 * @brief Load the task
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param tid The task id to load
 *
 * @return The task object, NULL otherwise
 */
task* M2TaskLoad(module *m, pool *p, unsigned int tid) {
  unsigned int i = 0, j = 0, k = 0, l = 0;
  size_t len;
  task* t = NULL;

  /* Allocate the task pointer */
  t = calloc(1, sizeof(task));
  if (!t) Error(CORE_ERR_MEM);

  t->pid = p->pid;
  t->tid = tid;
  t->rid = 0;
  t->cid = 0;
  t->node = p->node;

  t->storage = calloc(m->layer->init->banks_per_task, sizeof(storage));
  if (!t->storage) Error(CORE_ERR_MEM);

  /* Initialize task banks */
  for (i = 0; i < m->layer->init->banks_per_task; i++) {
    t->storage[i].layout = (schema) STORAGE_END;
    t->storage[i].memory = NULL;

    t->storage[i].compound_fields = 0;
    t->storage[i].attr_banks = 0;

    t->storage[i].attr = calloc(m->layer->init->attr_per_dataset, sizeof(attr));
    if (!t->storage[i].attr) Error(CORE_ERR_MEM);
    for (j = 0; j < m->layer->init->attr_per_dataset;  j++) {
      t->storage[i].attr[j].layout = (schema) ATTR_STORAGE_END;
      t->storage[i].attr[j].memory = NULL;
    }

    t->storage[i].field = calloc(m->layer->init->compound_fields, sizeof(field));
    if (!t->storage[i].field) Error(CORE_ERR_MEM);
    for (j = 0; j < m->layer->init->compound_fields; j++) {
      t->storage[i].field[j].layout = (schema) FIELD_STORAGE_END;
    }
  }

  /* Initialize the task */
  for (i = 0; i < p->task_banks; i++) {
    t->storage[i].layout.use_hdf = p->task->storage[i].layout.use_hdf;

    if (p->task->storage[i].layout.name != NULL) {
      len = strlen(p->task->storage[i].layout.name);
      t->storage[i].layout.name = calloc(len + 1, sizeof(char));
      if (!t->storage[i].layout.name) Error(CORE_ERR_MEM);

      strncpy(t->storage[i].layout.name, p->task->storage[i].layout.name, len);
      t->storage[i].layout.name[len] = CONFIG_NULL;
    }

    t->storage[i].layout.rank = p->task->storage[i].layout.rank;

    /* Memory size */
    for (j = 0; j < t->storage[i].layout.rank; j++) {
      t->storage[i].layout.storage_dim[j] =
        t->storage[i].layout.dims[j] =
        p->task->storage[i].layout.dims[j];
    }

    t->storage[i].layout.sync             = p->task->storage[i].layout.sync;
    t->storage[i].layout.storage_type     = p->task->storage[i].layout.storage_type;
    t->storage[i].layout.dataspace        = p->task->storage[i].layout.dataspace;
    t->storage[i].layout.datatype         = p->task->storage[i].layout.datatype;
    t->storage[i].layout.mpi_datatype     = p->task->storage[i].layout.mpi_datatype;
    t->storage[i].layout.size             = p->task->storage[i].layout.size;
    t->storage[i].layout.storage_size     = p->task->storage[i].layout.size;
    t->storage[i].layout.elements         = p->task->storage[i].layout.elements;
    t->storage[i].layout.storage_elements = p->task->storage[i].layout.elements;
    t->storage[i].layout.datatype_size    = p->task->storage[i].layout.datatype_size;
    t->storage[i].layout.compound_size    = p->task->storage[i].layout.compound_size;
    t->storage[i].attr_banks              = p->task->storage[i].attr_banks;
    t->storage[i].compound_fields         = p->task->storage[i].compound_fields;

    /**
     * Compound fields
     */
    for (j = 0; j < t->storage[i].compound_fields; j++) {
      if (p->task->storage[i].field[j].layout.datatype > 0) {
        t->storage[i].field[j].layout.datatype =
          p->task->storage[i].field[j].layout.datatype;
        t->storage[i].field[j].layout.mpi_datatype =
          p->task->storage[i].field[j].layout.mpi_datatype;
        t->storage[i].field[j].layout.size =
          p->task->storage[i].field[j].layout.size;
        t->storage[i].field[j].layout.storage_size =
          p->task->storage[i].field[j].layout.storage_size;
        t->storage[i].field[j].layout.datatype_size =
          p->task->storage[i].field[j].layout.datatype_size;
        t->storage[i].field[j].layout.elements =
          p->task->storage[i].field[j].layout.elements;
        t->storage[i].field[j].layout.storage_elements =
          p->task->storage[i].field[j].layout.elements;
        t->storage[i].field[j].layout.field_offset =
          p->task->storage[i].field[j].layout.field_offset;
        t->storage[i].field[j].layout.rank =
          p->task->storage[i].field[j].layout.rank;

        for (k = 0; k < MAX_RANK; k++) {
          t->storage[i].field[j].layout.dims[k] =
            p->task->storage[i].field[j].layout.dims[k];
        }

        // field name
        len = strlen(p->task->storage[i].field[j].layout.name);
        t->storage[i].field[j].layout.name = calloc(len + 1, sizeof(char));
        if (!t->storage[i].field[j].layout.name) Error(CORE_ERR_MEM);

        strncpy(t->storage[i].field[j].layout.name, p->task->storage[i].field[j].layout.name, len);
        t->storage[i].field[j].layout.name[len] = CONFIG_NULL;
      }
    }

    /**
     * Attributes
     */

    for (j = 0; j < t->storage[i].attr_banks; j++) {

      if (p->task->storage[i].attr[j].layout.name != NULL) {
        len = strlen(p->task->storage[i].attr[j].layout.name);
        t->storage[i].attr[j].layout.name = calloc(len + 1, sizeof(char));
        if (!t->storage[i].attr[j].layout.name) Error(CORE_ERR_MEM);

        strncpy(t->storage[i].attr[j].layout.name, p->task->storage[i].attr[j].layout.name, len);
        t->storage[i].attr[j].layout.name[len] = CONFIG_NULL;
      }

      t->storage[i].attr[j].layout.rank = p->task->storage[i].attr[j].layout.rank;

      for (k = 0; k < t->storage[i].attr[j].layout.rank; k++) {
        t->storage[i].attr[j].layout.storage_dim[k] =
          t->storage[i].attr[j].layout.dims[k] =
          p->task->storage[i].attr[j].layout.dims[k];
      }

      t->storage[i].attr[j].layout.sync             = p->task->storage[i].attr[j].layout.sync;
      t->storage[i].attr[j].layout.storage_type     = p->task->storage[i].attr[j].layout.storage_type;
      t->storage[i].attr[j].layout.dataspace        = p->task->storage[i].attr[j].layout.dataspace;
      t->storage[i].attr[j].layout.datatype         = p->task->storage[i].attr[j].layout.datatype;
      t->storage[i].attr[j].layout.mpi_datatype     = p->task->storage[i].attr[j].layout.mpi_datatype;
      t->storage[i].attr[j].layout.size             = p->task->storage[i].attr[j].layout.size;
      t->storage[i].attr[j].layout.storage_size     = p->task->storage[i].attr[j].layout.storage_size;
      t->storage[i].attr[j].layout.elements         = p->task->storage[i].attr[j].layout.elements;
      t->storage[i].attr[j].layout.storage_elements = p->task->storage[i].attr[j].layout.storage_elements;
      t->storage[i].attr[j].layout.datatype_size    = p->task->storage[i].attr[j].layout.datatype_size;
      t->storage[i].attr[j].compound_fields         = p->task->storage[i].attr[j].compound_fields;

        // Fields
        for (k = 0; k < t->storage[i].attr[j].compound_fields; k++) {
          if (p->task->storage[i].attr[j].field[k].layout.datatype > 0) {
          t->storage[i].attr[j].field[k].layout.datatype =
            p->task->storage[i].attr[j].field[k].layout.datatype;
          t->storage[i].attr[j].field[k].layout.mpi_datatype =
            p->task->storage[i].attr[j].field[k].layout.mpi_datatype;
          t->storage[i].attr[j].field[k].layout.size =
            p->task->storage[i].attr[j].field[k].layout.size;
          t->storage[i].attr[j].field[k].layout.storage_size =
            p->task->storage[i].attr[j].field[k].layout.storage_size;
          t->storage[i].attr[j].field[k].layout.datatype_size =
            p->task->storage[i].attr[j].field[k].layout.datatype_size;
          t->storage[i].attr[j].field[k].layout.elements =
            p->task->storage[i].attr[j].field[k].layout.elements;
          t->storage[i].attr[j].field[k].layout.storage_elements =
            p->task->storage[i].attr[j].field[k].layout.elements;
          t->storage[i].attr[j].field[k].layout.field_offset =
            p->task->storage[i].attr[j].field[k].layout.field_offset;
          t->storage[i].attr[j].field[k].layout.rank =
            p->task->storage[i].attr[j].field[k].layout.rank;

          for (l = 0; l < MAX_RANK; l++) {
            t->storage[i].attr[j].field[k].layout.dims[l] =
              p->task->storage[i].attr[j].field[k].layout.dims[l];
          }

          // field name
          len = strlen(p->task->storage[i].attr[j].field[k].layout.name);
          t->storage[i].attr[j].field[k].layout.name = calloc(len + 1, sizeof(char));
          if (!t->storage[i].attr[j].field[k].layout.name) Error(CORE_ERR_MEM);

          strncpy(t->storage[i].attr[j].field[k].layout.name, p->task->storage[i].attr[j].field[k].layout.name, len);
          t->storage[i].attr[j].field[k].layout.name[len] = CONFIG_NULL;
        }
      }

    }
  }

  CommitMemoryLayout(p->task_banks, t->storage);

  /* Copy attributes from p->task to the task object */
  for (i = 0; i < p->task_banks; i++) {
    for (j = 0; j < t->storage[i].attr_banks; j++) {
      CopyData(p->task->storage[i].attr[j].memory, t->storage[i].attr[j].memory,
          p->task->storage[i].attr[j].layout.size);
    }
  }

  return t;
}

/**
 * @brief Gets the ID of the available task
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param t The current task pointer
 * @param board_buffer The task board buffer
 *
 * @return 0 on success or NO_MORE_TASKS when the task board is finished
 */
int GetNewTask(module *m, pool *p, task *t, short ****board_buffer) {
  int mstat = SUCCESS;
  int x, y, z;
  query *q;

  while(1) {
    if (t->tid >= p->pool_size) return NO_MORE_TASKS;

    q = LoadSym(m, "TaskBoardMap", LOAD_DEFAULT);
    if (q) mstat = q(p, t);
    CheckStatus(mstat);

    x = t->location[0];
    y = t->location[1];
    z = t->location[2];

    if (m->mode == RESTART_MODE) {
      // Prepare the checkpoint data
      if (board_buffer[x][y][z][0] == TASK_TO_BE_RESTARTED) {
        mstat = TaskRestore(m, p, t);
        t->cid = board_buffer[x][y][z][2];
      }

      if (board_buffer[x][y][z][0] == TASK_AVAILABLE
          || board_buffer[x][y][z][0] == TASK_TO_BE_RESTARTED) break;
    }

    if (board_buffer[x][y][z][0] == TASK_AVAILABLE) {
      TaskReset(m, p, t, t->tid);
      t->status = TASK_IN_USE;
      break;
    }
    t->tid++;

  }

  return mstat;
}

/**
 * @brief Restore the task data from the pool data (restart mode)
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param t The task pointer
 *
 * @return 0 on success, error code otherwise
 */
int TaskRestore(module *m, pool *p, task *t) {
  int mstat = SUCCESS;
  unsigned int j = 0, k = 0, l = 0, r = 0;
  unsigned int e_offset = 0, l_offset = 0, k_offset = 0, z_offset = 0;
  unsigned int s_offset = 0, r_offset = 0, dim_offset = 0;
  size_t elements = 0;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];

  for (l = 0; l < MAX_RANK; l++) {
    dims[l] = p->board->layout.dims[l];
  }

  for (j = 0; j < p->task_banks; j++) {

    for (l = 0; l < MAX_RANK; l++) {
      offsets[l] = 0;
    }

    elements = 1;
    for (k = 1; k < t->storage[j].layout.rank; k++) {
      elements *= t->storage[j].layout.dims[k];
    }
    elements *= t->storage[j].layout.datatype_size;

    dim_offset = 1;
    for (k = 2; k < t->storage[j].layout.rank; k++) {
      dim_offset *= t->storage[j].layout.dims[k];
    }

    // Prepare STORAGE_PM3D
    if (t->storage[j].layout.storage_type == STORAGE_PM3D) {
      offsets[0] = (t->location[0] + dims[0]*t->location[1]) * t->storage[j].layout.dims[0]
        + t->location[2]*dims[0]*dims[1]*t->storage[j].layout.dims[0];
      offsets[1] = 0;
      Message(MESSAGE_DEBUG, "[%s:%d] PM3D[%d] task %d %d %d with offsets %d %d %d\n", __FILE__, __LINE__,
          j, t->tid, t->location[0], t->location[1], (int)offsets[0], (int)offsets[1], (int)offsets[2]);

      l_offset = elements;
      z_offset = 0;
    }

    // Prepare STORAGE_LIST
    if (t->storage[j].layout.storage_type == STORAGE_LIST) {
      offsets[0] = t->tid * t->storage[j].layout.dims[0];
      offsets[1] = 0;
      Message(MESSAGE_DEBUG, "[%s:%d] LIST[%d] task %d %d %d with offsets %d %d %d\n", __FILE__, __LINE__,
          j, t->tid, t->location[0], t->location[1], (int)offsets[0], (int)offsets[1], (int)offsets[2]);

      l_offset = elements;
      z_offset = 0;
    }

    // Prepare STORAGE_TEXTURE
    if (t->storage[j].layout.storage_type == STORAGE_TEXTURE) {
      offsets[0] = t->location[0] * t->storage[j].layout.dims[0];
      offsets[1] = t->location[1] * t->storage[j].layout.dims[1];
      offsets[2] = t->location[2] * t->storage[j].layout.dims[2];

      Message(MESSAGE_DEBUG, "[%s:%d] BOARD[%d] task %d %d %d with offsets %d %d %d\n", __FILE__, __LINE__,
          j, t->tid, t->location[0], t->location[1], (int)offsets[0], (int)offsets[1], (int)offsets[2]);

    }

    for (l = 0; l < MAX_RANK; l++) {
      t->storage[j].layout.offsets[l] = offsets[l];
    }

    // Restore the data
    if (t->storage[j].layout.storage_type == STORAGE_TEXTURE) {

      elements = 1;
      for (k = 2; k < t->storage[j].layout.rank; k++) {
        elements *= t->storage[j].layout.dims[k];
      }

      elements *= t->storage[j].layout.datatype_size;
      s_offset = dims[1] * dims[2] * elements * t->storage[j].layout.dims[1];

      for (k = 0; k < t->storage[j].layout.dims[0]; k++) {

        k_offset = k * s_offset;
        k_offset += t->location[0] * t->storage[j].layout.dims[0] * s_offset;
        k_offset += t->location[1] * t->storage[j].layout.dims[1] * dims[2] * elements;
        k_offset += t->location[2] * elements;

        for (r = 0; r < t->storage[j].layout.dims[1]; r++) {

          e_offset = r * elements + k * t->storage[j].layout.dims[1] * elements;
          r_offset = r * elements * dims[2] + k_offset;

          mstat = CopyData(p->task->storage[j].memory + r_offset, t->storage[j].memory + e_offset, elements);
          CheckStatus(mstat);
        }
      }
    } else if (t->storage[j].layout.storage_type == STORAGE_LIST ||
        t->storage[j].layout.storage_type == STORAGE_PM3D) {

      for (k = 0; k < t->storage[j].layout.dims[0]; k++) {
        k_offset = k * l_offset;
        k_offset += t->storage[j].layout.offsets[1] * dim_offset * t->storage[j].layout.datatype_size;
        k_offset += t->storage[j].layout.offsets[0] * l_offset;
        k_offset += z_offset;

        e_offset = k * elements;

        mstat = CopyData(p->task->storage[j].memory + k_offset, t->storage[j].memory + e_offset, elements);
        CheckStatus(mstat);
      }

    }

    if (t->storage[j].layout.storage_type == STORAGE_GROUP) {
      mstat = CopyData(p->tasks[t->tid]->storage[j].memory, t->storage[j].memory, t->storage[j].layout.size);
      CheckStatus(mstat);
    }
  }

  return mstat;
}

/**
 * @brief Prepare the task
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param t The current task pointer
 *
 * @return 0 on success, error code otherwise
 */
int M2TaskPrepare(module *m, pool *p, task *t) {
  int mstat = SUCCESS;
  query *q;

  q = LoadSym(m, "TaskPrepare", LOAD_DEFAULT);
  if (q) mstat = q(p, t);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief Process the task
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param t The current task pointer
 *
 * @return 0 on success, error code otherwise
 */
int M2TaskProcess(module *m, pool *p, task *t) {
  int mstat = SUCCESS;
  query *q;

  q = LoadSym(m, "TaskProcess", LOAD_DEFAULT);
  if (q) mstat = q(p, t);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief Reset the task, change the task ID
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param t The task to reset
 * @param tid The task new id
 */
void TaskReset(module *m, pool *p, task *t, unsigned int tid) {
  t->tid = tid;
  t->status = TASK_EMPTY;
  t->cid = 0;
  t->rid = 0;
}

/**
 * @brief Finalize the task
 *
 * @param m The module pointer
 * @param p The pool pointer
 * @param t The task pointer to be freed
 */
void TaskFinalize(module *m, pool *p, task *t) {
  unsigned int i = 0, j = 0, k = 0;

  if (t) {
    for (i = 0; i < p->task_banks; i++) {
      if (t->storage[i].layout.name) free(t->storage[i].layout.name);

      if (t->storage[i].attr) {
        for (j = 0; j < t->storage[i].attr_banks; j++) {
          for (k = 0; k < t->storage[i].attr[j].compound_fields; k++) {
            if (t->storage[i].attr[j].field[k].layout.name) free(t->storage[i].attr[j].field[k].layout.name);
          }
          if (t->storage[i].attr[j].layout.name) free(t->storage[i].attr[j].layout.name);
        }
      }

      if (t->storage[i].field) {
        for (j = 0; j < t->storage[i].compound_fields; j++) {
          if (t->storage[i].field[j].layout.name) free(t->storage[i].field[j].layout.name);
        }
      }
    }

    FreeMemoryLayout(p->task_banks, t->storage);

    if (t->storage) free(t->storage);
    free(t);
  }
}

