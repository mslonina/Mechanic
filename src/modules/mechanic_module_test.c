/**
 * @file
 * Default Mechanic module, implementation of all API functions
 */
#include "MMechanic2.h"
#include "mechanic_module_test.h"

/**
 * @function
 */
int Setup(setup s) {
  s.options[0] = (LRC_configDefaults) {"test", "step", "0.25", LRC_DOUBLE};
  s.options[1] = (LRC_configDefaults) {"test", "tend", "1000.0", LRC_DOUBLE};
  s.options[2] = (LRC_configDefaults) {"test", "driver", "1", LRC_INT};
  s.options[3] = (LRC_configDefaults) {"pool0", "size", "25", LRC_INT};
  s.options[4] = (LRC_configDefaults) {LRC_OPTIONS_END};

  return TASK_SUCCESS;
}
