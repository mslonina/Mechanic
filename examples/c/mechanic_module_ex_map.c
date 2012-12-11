/**
 * Sample map or image
 * ===================
 *
 * This example shows how to create a sample map (image) that is suitable to process with
 * Gnuplot PM3D
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_map.c -o libmechanic_module_ex_map.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_map -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 */
#include "mechanic.h"

/**
 * Implements Storage()
 *
 * Each worker will return 1x3 result array. The master node will combine the worker
 * result arrays into one dataset suitable to process with Gnuplot PM3D. The final dataset
 * will be available at /Pools/pool-0000/Tasks/result.
 *
 * By default, we have 8 memory/storage banks per each task pool + 8 memory/storage banks
 * per task available. You may change the defaults by using the Init() hook.
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .name = "input",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .sync = 0,
    .use_hdf = 0,
    .storage_type = STORAGE_PM3D,
    .datatype = H5T_NATIVE_DOUBLE
  };

  p->task->storage[1].layout = (schema) {
    .name = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
    .datatype = H5T_NATIVE_DOUBLE
  };
  return SUCCESS;
}

/**
 * Implements TaskProcess()
 *
 * For a dynamical map, we need to store the position of the point on the map and the
 * state of the dynamical system. In this example the position is represented by
 * task location, and the state of the system by the task unique identifier.
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double buffer_one[1][3];

  // The vertical position of the pixel
  buffer_one[0][0] = t->location[0];

  // The horizontal position of the pixel
  buffer_one[0][1] = t->location[1];

  // The state of the system
  buffer_one[0][2] = t->tid;

  MWriteData(t, "result", buffer_one);
  
  return SUCCESS;
}

