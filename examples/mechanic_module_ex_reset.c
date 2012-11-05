
#include "Mechanic2.h"

#define POOLS 4
#define REVISIONS 5

int Storage(pool *p, setup *s) {
  p->storage[0].layout = (schema) {
    .name = "global-double-data",
    .rank = 2,
    .dim[0] = 25,
    .dim[1] = 5,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_DOUBLE,
    .use_hdf = 1,
    .sync = 1
  };

  p->task->storage[0].layout = (schema) {
    .name = "task-integer-data",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 5,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT,
    .use_hdf = 1,
    .sync = 1
  };

  return SUCCESS;
}

int PoolPrepare(pool **all, pool *p, setup *s) {
  Message(MESSAGE_OUTPUT, "Revision ID: %d\n", p->rid);
  return SUCCESS;
}

int TaskProcess(pool *p, task *t, setup *s) {
  return SUCCESS;
}

int PoolProcess(pool *all, pool *p, setup *s) {
  if (p->rid <= REVISIONS) return POOL_RESET;
  if (p->pid <= POOLS) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}
