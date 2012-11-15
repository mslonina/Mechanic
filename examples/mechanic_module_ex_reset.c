/**
 * Using POOL_RESET
 * ================
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 -lhdf5 -lhdf5_hl mechanic_module_reset.c -o libmechanic_module_reset.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p reset -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 */
#include "Mechanic2.h"

#define POOLS 4
#define REVISIONS 5

/**
 * Implements Storage();
 */
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
    .storage_type = STORAGE_LIST,
    .datatype = H5T_NATIVE_INT,
    .use_hdf = 1,
    .sync = 1
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 *
 * The data of all previous pools is available through **all pointer, i.e.
 *
 *     all[0]->storage[0]
 *
 * contains data for first storage bank of the pool-0000. In this example, we take data of
 * the previous pool and put into the current one.
 *
 * The current pool is available through the p pointer. If the pool has been reset,
 * storage banks contains all previous data. That is, you may use these data to prepare
 * the better revision of the pool.
 */
int PoolPrepare(pool **all, pool *p, setup *s) {
  double **data;
  int dims[MAX_RANK], i = 0, j = 0;

  GetDims(&p->storage[0], dims);
  data = AllocateDouble2D(&p->storage[0]);

  Message(MESSAGE_OUTPUT, "Revision ID: %d\n", p->rid);

  /* Read data from the previous pool and modify it */
  if (p->pid > 0) {
    ReadData(&all[p->pid-1]->storage[0], &data[0][0]);
    for (i = 0; i < dims[0]; i++) {
      for (j = 0; j < dims[1]; j++) {
        data[i][j] += 2;
      }
    }
  } else {
    for (i = 0; i < dims[0]; i++) {
      for (j = 0; j < dims[1]; j++) {
        data[i][j] = p->pid - 1;
      }
    }
  }
  
  /* Write data to the storage buffer */
  WriteData(&p->storage[0], &data[0][0]);

  /* Release the resources */
  FreeDouble2D(data);

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 *
 * As a simple example, each worker will return simple result: the task id, the pool id
 * and the reset id
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int **buffer;

  buffer = AllocateInt2D(&t->storage[0]);

  buffer[0][0] = t->tid;
  buffer[0][1] = p->pid;
  buffer[0][2] = p->rid;

  WriteData(&t->storage[0], &buffer[0][0]);

  FreeInt2D(buffer);

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 *
 * Each pool will be reset REVISIONS times (the data of the pool is kept until
 * next PoolPrepare(), so it may be reused). After REVISIONS we create new task pool.
 */
int PoolProcess(pool *all, pool *p, setup *s) {
  if (p->rid <= REVISIONS) return POOL_RESET;
  if (p->pid <= POOLS) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}

