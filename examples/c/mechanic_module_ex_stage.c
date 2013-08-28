/**
 * Using POOL_STAGE
 * ================
 *
 * This example shows how to create and use pool stages
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_stage.c -o libmechanic_module_ex_stage.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_stage -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 */
#include "mechanic.h"

#define POOLS 4
#define REVISIONS 7
#define STAGES 3
#define STAGES_REVISIONS 5

/**
 * Implements Init()
 */
int Init(init *i) {
  i->pools = POOLS;
  return SUCCESS;
}

/**
 * Implements Storage();
 */
int Storage(pool *p) {
  p->storage[0].layout = (schema) {
    .name = "global-double-data",
    .rank = 2,
    .dims[0] = 25,
    .dims[1] = 5,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_DOUBLE,
    .use_hdf = 1,
    .sync = 1
  };

  p->task->storage[0].layout = (schema) {
    .name = "task-integer-data",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = 5,
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
int PoolPrepare(pool **all, pool *p) {
  double **data;
  unsigned int dims[MAX_RANK], i = 0, j = 0;

  MGetDims(p,"global-double-data", dims);
  MAllocate2(p, "global-double-data", data, double);

  Message(MESSAGE_OUTPUT, "Revision ID: %d Stage: %d\n", p->rid, p->sid);

  /* Read data from the previous pool and modify it */
  if (p->pid > 0) {
    MReadData(all[p->pid-1], "global-double-data", &data[0][0]);
    for (i = 0; i < dims[0]; i++) {
      for (j = 0; j < dims[1]; j++) {
        data[i][j] += 2.0;
      }
    }
  } else {
    for (i = 0; i < dims[0]; i++) {
      for (j = 0; j < dims[1]; j++) {
        data[i][j] = i + j + p->pid - 1.0;
      }
    }
  }
  
  /* Write data to the storage buffer */
  MWriteData(p, "global-double-data", &data[0][0]);

  /* Release the resources */
  free(data);

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 *
 * As a simple example, each worker will return simple result: the task id, the pool id
 * and the reset id
 */
int TaskProcess(pool *p, task *t) {
  int **buffer;

  MAllocate2(t, "task-integer-data", buffer, int);

  buffer[0][0] = t->tid;
  buffer[0][1] = p->pid;
  buffer[0][2] = p->rid;
  buffer[0][3] = p->sid;
  buffer[0][4] = p->srid;

  MWriteData(t, "task-integer-data", &buffer[0][0]);

  free(buffer);

  return TASK_FINALIZE;
}

/**
 * Implements PoolProcess()
 *
 * Each pool will be reset REVISIONS times (the data of the pool is kept until
 * next PoolPrepare(), so it may be reused). After REVISIONS we create new task pool.
 */
int PoolProcess(pool **all, pool *p) {

  Message(MESSAGE_INFO, "Stage %d rev. %d completed\n", p->sid, p->srid);

  if (p->srid <= STAGES_REVISIONS) return POOL_STAGE_RESET;
  if (p->sid <= STAGES) return POOL_STAGE;
  if (p->rid <= REVISIONS) return POOL_RESET;
  if (p->pid <= POOLS) return POOL_CREATE_NEW;

  return POOL_FINALIZE;
}

