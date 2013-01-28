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
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_chpoollayout2.c -o libmechanic_module_ex_chpoollayout2.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_chpoollayout2 -x 10 -y 20
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
#include "mechanic.h"

/**
 * Implements Storage()
 *
 * Each worker will return 1x3 result array. The master node will combine the worker
 * result arrays into one dataset suitable to process with Gnuplot PM3D. The final dataset
 * will be available at /Pools/pool-ID/Tasks/result.
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .name = "result",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = 3,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
    .datatype = H5T_NATIVE_DOUBLE
  };

  // At the pool-0002 we add some more datasets
  // The result arrays are 3x3 and the storage will follow the task board layout:
  // The data will be combined into one dataset of dimensions x*3 x y*3, and each worker
  // result will have proper offsets
  if (p->pid == 2) {
    p->task->storage[1].layout = (schema) {
      .name = "result-board",
      .rank = TASK_BOARD_RANK,
      .dims[0] = 3,
      .dims[1] = 3,
      .dims[2] = 1,
      .sync = 1,
      .use_hdf = 1,
      .storage_type = STORAGE_BOARD,
      .datatype = H5T_NATIVE_DOUBLE
    };
  }

  // Change the layout at the pool-0004
  if (p->pid == 4) {
    p->task->storage[0].layout.dims[1] = 5;
    // Each task will have its own result dataset
    // /Pools/pool-0004/Tasks/task-ID/result
    p->task->storage[0].layout.storage_type = STORAGE_GROUP;
  }

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int i,j;
  double buffer_one[1][3];
  double buffer_two[3][3];
  double buffer_three[1][5];

  // The vertical position of the pixel
  buffer_one[0][0] = t->location[0];

  // The horizontal position of the pixel
  buffer_one[0][1] = t->location[1];

  // The state of the system
  buffer_one[0][2] = t->tid;

  // We are at pool-0002
  if (p->pid == 2) {
    for (i = 0; i < t->storage[1].layout.dims[0]; i++) {
      for (j = 0; j < t->storage[1].layout.dims[1]; j++) {
        buffer_two[i][j] = i+j;
      }
    }
    MWriteData(t, "result-board", &buffer_two[0][0]);
  }

  // We are at pool-0004
  if (p->pid == 4) {
    buffer_three[0][0] = t->location[0];
    buffer_three[0][1] = t->location[1];
    buffer_three[0][2] = t->tid;
    buffer_three[0][3] = t->tid + 3.1;
    buffer_three[0][4] = t->tid + 4.1;

    MWriteData(t, "result", &buffer_three[0][0]);
  } else {
    MWriteData(t, "result", &buffer_one[0][0]);
  }

  return TASK_FINALIZE;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  int i,j;
  double buffer[1][5];

  // Access the stored data in the current pool for STORAGE_GROUP
  if (current->pid == 4) {
    for (i = 0; i < current->pool_size; i++) {
      MReadData(current->tasks[i], "result", &buffer[0][0]);
      Message(MESSAGE_OUTPUT, "task[%04d] = [%04d %04d %04d]\n",
       current->tasks[i]->tid,
       (int)buffer[0][0],
       (int)buffer[0][1],
       (int)buffer[0][2]
      );
    }
  }

  if (current->pid < 5) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}

