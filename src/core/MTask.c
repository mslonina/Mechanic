/**
 * @file
 * The task related functions
 */
#include "MTask.h"

/**
 * @function
 * Load the task
 */
task* TaskLoad(module *m, pool *p, int tid) {
  task* t = NULL;
  int i = 0, j = 0;
  size_t len;

  /* Allocate the task pointer */
  t = calloc(sizeof(task), sizeof(task));
  if (!t) Error(CORE_ERR_MEM);

  t->pid = p->pid;
  t->tid = tid;

  t->storage = calloc(m->layer.init.banks_per_task * sizeof(storage), sizeof(storage));
  if (!t->storage) Error(CORE_ERR_MEM);

  /* Initialize task banks */
  for (i = 0; i < m->layer.init.banks_per_task; i++) {
    t->storage[i].layout = (schema) STORAGE_END;
    t->storage[i].data = NULL;
  }

  /* Initialize the task */
  for (i = 0; i < m->task_banks; i++) {
    t->storage[i].layout.dataspace_type = p->task->storage[i].layout.dataspace_type;
    t->storage[i].layout.datatype = p->task->storage[i].layout.datatype;
    t->storage[i].layout.rank = p->task->storage[i].layout.rank;
    t->storage[i].layout.use_hdf = p->task->storage[i].layout.use_hdf;
    t->storage[i].layout.sync = p->task->storage[i].layout.sync;
    t->storage[i].layout.storage_type = p->task->storage[i].layout.storage_type;

    /* Memory size */
    for (j = 0; j < t->storage[i].layout.rank; j++) {
      t->storage[i].layout.dim[j] = p->task->storage[i].layout.dim[j];
    }

    /* Setup path, we need this only when use_hdf = 1 */
    if (t->storage[i].layout.use_hdf) {
      if (p->task->storage[i].layout.path != NULL) {
        len = strlen(p->task->storage[i].layout.path);
        t->storage[i].layout.path = calloc((len+1) * sizeof(char), sizeof(char));
        if (!t->storage[i].layout.path) Error(CORE_ERR_MEM);

        strncpy(t->storage[i].layout.path, p->task->storage[i].layout.path, len);
        t->storage[i].layout.path[len] = LRC_NULL;
      }
    }
  }

  CommitMemoryLayout(m->task_banks, t->storage);

  return t;
}

/**
 * @function
 * Prepare the task
 */
int TaskPrepare(module *m, pool *p, task *t) {
  int mstat = 0;
  int x, y;
  query *q;
  setup *s = &(m->layer.setup);

  if (m->node == MASTER) {

    /* Get the ID of the available task */
    while(1) {
      if (t->tid >= p->pool_size) return NO_MORE_TASKS;

      q = LoadSym(m, "TaskMapping", LOAD_DEFAULT);
      if (q) mstat = q(p, t, s);

      x = t->location[0];
      y = t->location[1];

      if (p->board->data[x][y] == (double) TASK_AVAILABLE) break;
      t->tid++;

    }

    q = LoadSym(m, "TaskPrepare", LOAD_DEFAULT);
    if (q) mstat = q(p, t, s);
  }

  return mstat;
}

/**
 * @function
 * Process the task
 */
int TaskProcess(module *m, pool *p, task *t) {
  int mstat = 0;
  query *q;
  setup *s = &(m->layer.setup);

  q = LoadSym(m, "TaskProcess", LOAD_DEFAULT);
  if (q) mstat = q(p, t, s);

  return mstat;
}

/**
 * @function
 */
void TaskReset(module *m, pool *p, task *t, int tid) {
  t->status = TASK_EMPTY;
}

/**
 * @function
 * Finalize the task
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

