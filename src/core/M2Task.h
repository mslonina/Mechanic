/**
 * @file
 * Public task interface
 */
#ifndef MECHANIC_PUBLIC_TASK_H
#define MECHANIC_PUBLIC_TASK_H

/* Task */
#define TASK_EMPTY -88 /**< The task empty return code */
#define TASK_FINISHED 1 /**< The task finished return code */
#define TASK_AVAILABLE 0 /**< The task available return code */
#define TASK_IN_USE -1 /**< The task in use return code */
#define TASK_TO_BE_RESTARTED -2 /**< The task to be restarted return code */
#define NO_MORE_TASKS -99 /**< No more tasks return code */
#define TASK_FINALIZE 3001 /**< The task finalize return code */
#define TASK_RESET 3002 /**< The task return return code */
#define TASK_CREATE_NEW 3003 /**< The task create new return code */
#define TASK_CHECKPOINT 3004 /**< The task checkpoint return code */
#define TASK_NO_LOCATION -99 /**< Task location defaults */

/**
 * @struct task
 * The task
 */
typedef struct {
  int pid; /**< The parent pool id */
  int tid; /**< The task id */
  int rid; /**< The task reset id */
  int cid; /**< The task checkpoint id */
  int status; /**< The task status */
  int state; /**< The task processing state */
  int location[TASK_BOARD_RANK]; /**< Coordinates of the task */
  short node; /** The computing node */
  storage *storage; /**< The storage schema and data */
} task;

int ReadTask(task *t, char *storage_name, void *data); /**< Read task data */
int WriteTask(task *t, char *storage_name, void *data); /**< Write data to the task */

#endif

