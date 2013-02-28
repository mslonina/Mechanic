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
task* M2TaskLoad(module *m, pool *p, int tid) {
  int i = 0, j = 0;
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

  t->storage = calloc(m->layer.init.banks_per_task, sizeof(storage));
  if (!t->storage) Error(CORE_ERR_MEM);

  /* Initialize task banks */
  for (i = 0; i < m->layer.init.banks_per_task; i++) {
    t->storage[i].layout = (schema) STORAGE_END;
    t->storage[i].memory = NULL;
    t->storage[i].attr = calloc(m->layer.init.attr_per_dataset, sizeof(storage));
    if (!t->storage[i].attr) Error(CORE_ERR_MEM);
    for (j = 0; j < m->layer.init.attr_per_dataset;  j++) {
      t->storage[i].attr[j].layout = (schema) ATTR_STORAGE_END;
      t->storage[i].attr[j].memory = NULL;
    }
  }

  /* Initialize the task */
  for (i = 0; i < p->task_banks; i++) {
    t->storage[i].layout.use_hdf = p->task->storage[i].layout.use_hdf;

    /* The dataset name, we need this only when use_hdf = 1 */
    if (t->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.name != NULL) {
        len = strlen(p->task->storage[i].layout.name);
        t->storage[i].layout.name = calloc(len+1, sizeof(char));
        if (!t->storage[i].layout.name) Error(CORE_ERR_MEM);

        strncpy(t->storage[i].layout.name, p->task->storage[i].layout.name, len);
        t->storage[i].layout.name[len] = CONFIG_NULL;
      }
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
    
    /**
     * @todo Attributes
     *
     * If only worker nodes knew about attributes...
     */
  }

  CommitMemoryLayout(p->task_banks, t->storage);

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
  void *s = NULL;

  while(1) {
    if (t->tid >= p->pool_size) return NO_MORE_TASKS;

    q = LoadSym(m, "TaskBoardMap", LOAD_DEFAULT);
    if (q) mstat = q(p, t, s);
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
  int j = 0, k = 0, l = 0, r = 0;
  int e_offset = 0, l_offset = 0, k_offset = 0, z_offset = 0;
  int s_offset = 0, r_offset = 0, dim_offset = 0;
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
    
    // Prepare STORAGE_BOARD
    if (t->storage[j].layout.storage_type == STORAGE_BOARD) {
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
    if (t->storage[j].layout.storage_type == STORAGE_BOARD) {

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
  void *s = NULL;

  q = LoadSym(m, "TaskPrepare", LOAD_DEFAULT);
  if (q) mstat = q(p, t, s);
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
  void *s = NULL;

  q = LoadSym(m, "TaskProcess", LOAD_DEFAULT);
  if (q) mstat = q(p, t, s);
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
void TaskReset(module *m, pool *p, task *t, int tid) {
  t->tid = TASK_EMPTY;
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
  int i = 0, j = 0;

  for (i = 0; i < p->task_banks; i++) {
    if (t->storage[i].layout.use_hdf) {
      if (t->storage[i].layout.name) free(t->storage[i].layout.name);
    }
    for (j = 0; j < t->storage[i].attr_banks; j++) {
      if (t->storage[i].attr[j].layout.name) free(t->storage[i].attr[j].layout.name);
    }
  }

  FreeMemoryLayout(p->task_banks, t->storage);

  if (t->storage) free(t->storage);
  if (t) free(t);
}

