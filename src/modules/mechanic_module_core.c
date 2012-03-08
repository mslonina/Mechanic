/**
 * @file
 * Core Mechanic module, implementation of all API functions
 */

#include "MMechanic2.h"
#include "mechanic_module_core.h"

/**
 * @function
 * Initializes critical core variables 
 *
 * At least the core module implements this function. The user may override the core
 * defaults in custom module -- in such case, the module variables will be merged with
 * core.
 */
int Init(init *i) {
  i->options = 128;
  i->pools = 64;
  i->banks_per_pool = 8;
  i->banks_per_task = 8;
  
  return TASK_SUCCESS;
}

/**
 * @function
 * Define the default configuration options
 *
 * This is an implementation of the LRC API. We initialize here default options.
 * If the user-supplied module implements this function, the setup options will be merged
 * with the core. All options will be available in the setup->head linked list.
 */
int Setup(setup *s) {
  s->options[0] = (LRC_configDefaults) {"core", "name", "mechanic", LRC_STRING};
  s->options[1] = (LRC_configDefaults) {"core", "debug", "0", LRC_INT};
  s->options[2] = (LRC_configDefaults) {"core", "silent", "0", LRC_INT};
  s->options[3] = (LRC_configDefaults) {"core", "api", "2", LRC_INT};
  s->options[4] = (LRC_configDefaults) {"core", "hdf", "3", LRC_INT};
  s->options[5] = (LRC_configDefaults) {LRC_OPTIONS_END};
  
  return TASK_SUCCESS;
}

/**
 * @function
 * Define the pool storage layout
 *
 * If the user-supplied module implements this function, the layout will be merged with
 * the core.
 *
 * The Pool will store its global data in the /Pools/pool-ID. The task data will be stored
 * in /Pools/pool-ID/tasks. 
 *
 * The size of the storage dataset and the memory is the same. In the case of pool, whole
 * memory block is stored at once, if use_hdf = 1. In case of task, the storage dataset
 * has the size of dim[0]*number-of-tasks, and each task-dataset is stored with the offset
 * of dim[0]*task-ID.
 *
 * The Pool data is broadcasted right after PoolPrepare() and saved to the master
 * datafile.
 */
int Storage(pool *p, setup *s) {

  /* Path: /Pools/pool-ID/master */
  p->storage[0].layout.path = "master";
  p->storage[0].layout.dataspace_type = H5S_SIMPLE;
  p->storage[0].layout.datatype = H5T_NATIVE_DOUBLE;
  p->storage[0].layout.rank = 2;
  p->storage[0].layout.dim[0] = 23;
  p->storage[0].layout.dim[1] = 7;
  p->storage[0].layout.use_hdf = 1;
  
  /* Path: /Pools/pool-ID/tmp */
  p->storage[1].layout.path = "tmp";
  p->storage[1].layout.dataspace_type = H5S_SIMPLE;
  p->storage[1].layout.datatype = H5T_NATIVE_DOUBLE;
  p->storage[1].layout.rank = 2;
  p->storage[1].layout.dim[0] = 43;
  p->storage[1].layout.dim[1] = 7;
  p->storage[1].layout.use_hdf = 0;

  /* Path: /Pools/pool-ID/tasks/masterdata */
  p->task.storage[0].layout.path = "masterdata";
  p->task.storage[0].layout.dataspace_type = H5S_SIMPLE;
  p->task.storage[0].layout.datatype = H5T_NATIVE_DOUBLE;
  p->task.storage[0].layout.rank = 2;
  p->task.storage[0].layout.dim[0] = 1;
  p->task.storage[0].layout.dim[1] = 12;
  p->task.storage[0].layout.use_hdf = 1;

  /* Path: /Pools/pool-ID/tasks/tmpdata */
  p->task.storage[1].layout.path = "tmpdata";
  p->task.storage[1].layout.dataspace_type = H5S_SIMPLE;
  p->task.storage[1].layout.datatype = H5T_NATIVE_DOUBLE;
  p->task.storage[1].layout.rank = 2;
  p->task.storage[1].layout.dim[0] = 10;
  p->task.storage[1].layout.dim[1] = 12;
  p->task.storage[1].layout.use_hdf = 0;

  return TASK_SUCCESS;
}

/**
 * @function
 * Postprocess the pool
 *
 * @return
 * POOL_FINALIZE for the last pool or POOL_CREATE_NEW, if the pool loop have to continue
 */
int PoolPostprocess(pool *p, setup *s) {
  return POOL_FINALIZE;
}
