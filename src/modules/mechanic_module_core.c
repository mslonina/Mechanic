/**
 * @file
 * Default Mechanic module, implementation of all API functions
 */

#include "MMechanic2.h"
#include "mechanic_module_core.h"

/**
 * @function
 */
int Setup(setup s) {
  s.options[0] = (LRC_configDefaults) {"core", "name", "mechanic", LRC_STRING};
  s.options[1] = (LRC_configDefaults) {"core", "debug", "0", LRC_INT};
  s.options[2] = (LRC_configDefaults) {"core", "silent", "0", LRC_INT};

  return TASK_SUCCESS;
}
