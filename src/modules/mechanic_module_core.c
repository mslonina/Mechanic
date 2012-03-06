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
  i->datasets_per_pool = 64;
  
  return TASK_SUCCESS;
}

/**
 * @function
 * Define the default configuration options
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
 */
int Storage(pool *p, setup *s) {

  p->storage[0].path = "master";
  p->storage[0].type = H5S_SIMPLE;
  p->storage[0].datatype = H5T_NATIVE_DOUBLE;
  p->storage[0].rank = 2;
  p->storage[0].dimsf[0] = 23;
  p->storage[0].dimsf[1] = 7;
  p->storage[0].use_hdf = 1;
  
  p->storage[1].path = "tmp";
  p->storage[1].type = H5S_SIMPLE;
  p->storage[1].datatype = H5T_NATIVE_DOUBLE;
  p->storage[1].rank = 2;
  p->storage[1].dimsf[0] = 43;
  p->storage[1].dimsf[1] = 7;
  p->storage[1].use_hdf = 0;

  return TASK_SUCCESS;
}

/**
 * @function
 */
int PoolPostprocess(pool *p, setup *s) {
  return POOL_FINALIZE;
}
