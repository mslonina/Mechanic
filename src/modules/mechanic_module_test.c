/**
 * @file
 * Default Mechanic module, implementation of all API functions
 */
#include "MMechanic2.h"
#include "mechanic_module_test.h"

/**
 * @function
 */
int Init(init *i) {
  i->options = 123;
  i->pools = 61;
  
  return TASK_SUCCESS;
}

/**
 * @function
 */
int Setup(setup *s) {
  s->options[0] = (LRC_configDefaults) {"test", "step", "0.25", LRC_DOUBLE};
  s->options[1] = (LRC_configDefaults) {"test", "tend", "1000.0", LRC_DOUBLE};
  s->options[2] = (LRC_configDefaults) {"test", "driver", "1", LRC_INT};
  s->options[3] = (LRC_configDefaults) {"pool0", "size", "25", LRC_INT};
  s->options[5] = (LRC_configDefaults) {LRC_OPTIONS_END};

  return TASK_SUCCESS;
}

/**
 * @function
 */
int Storage(pool *p, setup *s) {

  p->storage[2].path = "conditions";
  p->storage[2].type = H5S_SIMPLE;
  p->storage[2].datatype = H5T_NATIVE_DOUBLE;
  p->storage[2].rank = 2;
  p->storage[2].dimsf[0] = 43;
  p->storage[2].dimsf[1] = 7;
  p->storage[2].use_hdf = 1;

  return TASK_SUCCESS;
}

/**
 * @function
 */
int PoolPostprocess(pool *p, setup *s) {
  if (p->pid > 2) return POOL_FINALIZE;
  return POOL_CREATE_NEW;
}
