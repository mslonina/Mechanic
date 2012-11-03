/**
 * @file
 * The task related functions
 */
#include "MTask.h"

/**
 * @brief Load the task
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param tid The task id to load
 *
 * @return The task object, NULL otherwise
 */
task* TaskLoad(module *m, pool *p, int tid) {
  int i = 0, j = 0;
  size_t len;
  task* t = NULL;

  /* Allocate the task pointer */
  t = calloc(1, sizeof(task));
  if (!t) Error(CORE_ERR_MEM);

  t->pid = p->pid;
  t->tid = tid;
  t->node = m->node;

  t->storage = calloc(m->layer.init.banks_per_task, sizeof(storage));
  if (!t->storage) Error(CORE_ERR_MEM);

  /* Initialize task banks */
  for (i = 0; i < m->layer.init.banks_per_task; i++) {
    t->storage[i].layout = (schema) STORAGE_END;
    t->storage[i].memory = NULL;
  }

  /* Initialize the task */
  for (i = 0; i < m->task_banks; i++) {
    t->storage[i].layout.use_hdf = p->task->storage[i].layout.use_hdf;

    /* The setup path, we need this only when use_hdf = 1 */
    if (t->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.path != NULL) {
        len = strlen(p->task->storage[i].layout.path);
        t->storage[i].layout.path = calloc(len+1, sizeof(char));
        if (!t->storage[i].layout.path) Error(CORE_ERR_MEM);

        strncpy(t->storage[i].layout.path, p->task->storage[i].layout.path, len);
        t->storage[i].layout.path[len] = LRC_NULL;
      }
    }

    t->storage[i].layout.rank = p->task->storage[i].layout.rank;

    /* Memory size */
    for (j = 0; j < t->storage[i].layout.rank; j++) {
      t->storage[i].layout.storage_dim[j] = 
        t->storage[i].layout.dim[j] = 
        p->task->storage[i].layout.dim[j];
    }

    t->storage[i].layout.sync = p->task->storage[i].layout.sync;
    t->storage[i].layout.storage_type = p->task->storage[i].layout.storage_type;
    t->storage[i].layout.dataspace_type = p->task->storage[i].layout.dataspace_type;
    t->storage[i].layout.datatype = p->task->storage[i].layout.datatype;
    t->storage[i].layout.mpi_datatype = p->task->storage[i].layout.mpi_datatype;
    t->storage[i].layout.size = p->task->storage[i].layout.size;
    t->storage[i].layout.storage_size = p->task->storage[i].layout.size;
    t->storage[i].layout.elements = p->task->storage[i].layout.elements;
    t->storage[i].layout.storage_elements = p->task->storage[i].layout.elements;
    t->storage[i].layout.datatype_size = p->task->storage[i].layout.datatype_size;
  }

  CommitMemoryLayout(m->task_banks, t->storage);

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
  setup *s = &(m->layer.setup);

  while(1) {
    if (t->tid >= p->pool_size) return NO_MORE_TASKS;

    q = LoadSym(m, "TaskMapping", LOAD_DEFAULT);
    if (q) mstat = q(p, t, s);
    CheckStatus(mstat);

    x = t->location[0];
    y = t->location[1];
    z = t->location[2];

    if (m->mode == RESTART_MODE) {
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
 * @brief Prepare the task
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param t The current task pointer
 *
 * @return 0 on success, error code otherwise
 */
int TaskPrepare(module *m, pool *p, task *t) {
  int mstat = SUCCESS;
  query *q;
  setup *s = &(m->layer.setup);

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
int TaskProcess(module *m, pool *p, task *t) {
  int mstat = SUCCESS;
  query *q;
  setup *s = &(m->layer.setup);

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
}

/**
 * @brief Finalize the task
 *
 * @param m The module pointer
 * @param p The pool pointer
 * @param t The task pointer to be freed
 */
void TaskFinalize(module *m, pool *p, task *t) {
  int i = 0;

  for (i = 0; i < m->task_banks; i++) {
    if (t->storage[i].layout.use_hdf) {
      if (t->storage[i].layout.path) free(t->storage[i].layout.path);
    }
  }

  FreeMemoryLayout(m->task_banks, t->storage);

  if (t->storage) free(t->storage);
  if (t) free(t);
}

