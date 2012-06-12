/**
 * Sample map or image
 * ===================
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 -lreadocnfig mechanic_module_map.c -o libmechanic_module_map.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p map -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 */
#include "MMechanic2.h"

/**
 * Implements Storage()
 *
 * Each worker will return 1x3 result array. The master node will combine the worker
 * result arrays into one dataset suitable to process with Gnuplot PM3D. The final dataset
 * will be available at /Pools/pool-0000/Tasks/result.
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .path = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
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

  // The horizontal position of the pixel
  t->storage[0].data[0][0] = t->location[0];

  // The vertical position of the pixel

  t->storage[0].data[0][1] = t->location[1];

  // The state of the system
  t->storage[0].data[0][2] = t->tid;

  return SUCCESS;
}

