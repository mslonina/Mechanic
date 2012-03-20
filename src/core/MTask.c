/**
 * @file
 * The task related functions.
 */
#include "MTask.h"
/**
 * @function
 */
task TaskLoad(module *m, pool *p, int tid) {
  task t;
  int i = 0, j = 0;
  size_t len;

  t.pid = p->pid;
  t.tid = tid;

  //t->storage = calloc(m->layer.init.banks_per_task * sizeof(storage*), sizeof(storage*));
  t.storage = (storage*) malloc(m->layer.init.banks_per_task * sizeof(storage));
  if (!t.storage) Error(CORE_ERR_MEM);

  for (i = 0; i < m->task_banks; i++) {
    /* Setup path */
    len = strlen(p->task->storage[i].layout.path);
    t.storage[i].layout.path = (char*) malloc((len+1) * sizeof(char));
    if (!t.storage[i].layout.path) Error(CORE_ERR_MEM);

    strncpy(t.storage[i].layout.path, p->task->storage[i].layout.path, len);
    t.storage[i].layout.path[len] = LRC_NULL;

    t.storage[i].layout.dataspace_type = p->task->storage[i].layout.dataspace_type;
    t.storage[i].layout.datatype = p->task->storage[i].layout.datatype;
    t.storage[i].layout.rank = p->task->storage[i].layout.rank;
    t.storage[i].layout.use_hdf = p->task->storage[i].layout.use_hdf;
    t.storage[i].layout.sync = p->task->storage[i].layout.sync;

    /* Memory size */
    for (j = 0; j < t.storage[i].layout.rank; j++) {
      t.storage[i].layout.dim[j] = p->task->storage[i].layout.dim[j];
    }
  }

  CommitMemoryLayout(m->task_banks, t.storage);

  return t;
}

/**
 * @function
 */
int TaskInit(module *m, pool *p, task *t) {
  int mstat = 0;

  return mstat;
}

/**
 * @function
 */
int TaskPrepare(module *m, pool *p, task *t) {
  int mstat = 0;
  query *q;
  setup s = m->layer.setup;

  if (m->node == MASTER) {
    t->location[0] = 27;
    t->location[1] = 43;
  }

  q = LoadSym(m, "TaskPrepare", LOAD_DEFAULT);
  if (q) mstat = q(p, t, s);

  return mstat;
}

/**
 * @function
 */
int TaskProcess(module *m, pool *p, task *t) {
  int mstat = 0;
  query *q;
  setup s = m->layer.setup;

  q = LoadSym(m, "TaskProcess", LOAD_DEFAULT);
  if (q) mstat = q(p, t, s);

  return mstat;
}

/**
 * @function
 */
void TaskFinalize(module *m, pool *p, task *t) {
  int i = 0;

  for (i = 0; i < m->task_banks; i++) {
    if (t->storage[i].layout.path) free(t->storage[i].layout.path);
  }
  FreeMemoryLayout(m->task_banks, t->storage);
  
  if (t->storage) free(t->storage);
}

