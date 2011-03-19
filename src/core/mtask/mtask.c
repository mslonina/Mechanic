#include "mtask.h"

/**
 * @function
 * Initialize the task
 *
 * @return task
 */
mtask* mtask_init() {
  mtask* task;

  return task;
}

/**
 * @function
 * Gets the status of the task
 *
 * @return status
 */
int mtask_get_status(mtask* mtask) {
  int status;

  return status;
}

/**
 * @function
 * Sets the status of the task
 */
int mtask_set_status(mtask* task, int status) {
  task->status = status;
}

/**
 * @function
 * Finalizes the task
 */
void mtask_cleanup(mstat* task) {

}
