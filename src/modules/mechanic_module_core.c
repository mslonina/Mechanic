/**
 * @file
 * Default Mechanic module, implementation of all API functions
 */

#include "MMechanic2.h"
#include "mechanic_module_core.h"

/**
 * @function
 */
int Init(init *i) {
  i->options = 128;
  i->pools = 64;
  
  return TASK_SUCCESS;
}

/**
 * @function
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
