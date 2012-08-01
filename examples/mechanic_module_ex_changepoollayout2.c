/**
 * Change the storage layout for a specific task pool (v2)
 * =======================================================
 *
 * In this example we create new task pools during the simulation, and we change the
 * storage layout for one of them. We complicate the example by adding some new datasets
 * and changing the storage types. At the end we do some basic data access.
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 mechanic_module_chpoollayout2.c -o libmechanic_module_chpoollayout2.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p chpoollayout2 -x 10 -y 20
 *
 * Listing the contents of the data file
 * -------------------------------------
 *
 *    h5ls -r mechanic-master-00.h5
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *    h5dump -d/Pools/pool-0001/Tasks/result mechanic-master-00.h5
 *    h5dump -d/Pools/pool-0002/Tasks/result mechanic-master-00.h5
 *    h5dump -d/Pools/pool-0003/Tasks/result mechanic-master-00.h5
 */
#include "Mechanic2.h"

/**
 * Implements Storage()
 *
 * Each worker will return 1x3 result array. The master node will combine the worker
 * result arrays into one dataset suitable to process with Gnuplot PM3D. The final dataset
 * will be available at /Pools/pool-ID/Tasks/result.
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .path = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };

  // At the pool-0002 we add some more datasets
  // The result arrays are 3x3 and the storage will follow the task board layout:
  // The data will be combined into one dataset of dimensions x*3 x y*3, and each worker
  // result will have proper offsets
  if (p->pid == 2) {
    p->task->storage[1].layout = (schema) {
      .path = "result-board",
      .rank = 2,
      .dim[0] = 3,
      .dim[1] = 3,
      .sync = 1,
      .use_hdf = 1,
      .storage_type = STORAGE_BOARD,
    };
  }

  // Change the layout at the pool-0004
  if (p->pid == 4) {
    p->task->storage[0].layout.dim[1] = 5;
    // Each task will have its own result dataset
    // /Pools/pool-0004/Tasks/task-ID/result
    p->task->storage[0].layout.storage_type = STORAGE_BASIC;
  }

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int i,j;

  // The vertical position of the pixel
  t->storage[0].data[0][0] = t->location[0];

  // The horizontal position of the pixel
  t->storage[0].data[0][1] = t->location[1];

  // The state of the system
  t->storage[0].data[0][2] = t->tid;

  // We are at pool-0002
  if (p->pid == 2) {
    for (i = 0; i < t->storage[1].layout.dim[0]; i++) {
      for (j = 0; j < t->storage[1].layout.dim[1]; j++) {
        t->storage[1].data[i][j] = i+j;
      }
    }
  }

  // We are at pool-0004
  if (p->pid == 4) {
    t->storage[0].data[0][3] = t->tid + 3.0;
    t->storage[0].data[0][4] = t->tid + 4.0;
  }

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  int i,j;
  // Access the stored data in the current pool for STORAGE_BASIC
  if (current->pid == 4) {
    for (i = 0; i < current->pool_size; i++) {
      Message(MESSAGE_OUTPUT, "task[%d] = [%d %d %d]\n",
       current->tasks[i]->tid,
       (int)current->tasks[i]->storage[0].data[0][0],
       (int)current->tasks[i]->storage[0].data[0][1],
       (int)current->tasks[i]->storage[0].data[0][2]
      );
    }
  }
  if (current->pid < 5) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}

