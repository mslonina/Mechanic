/**
 * @file
 * The task (public API)
 */
#ifndef MECHANIC_M2T_PUBLIC_H
#define MECHANIC_M2T_PUBLIC_H

#include "M2Apublic.h"
#include "M2Hpublic.h"
#include "M2Spublic.h"
#include "M2Ppublic.h"

/* Task */
#define TASK_EMPTY -88 /**< The task empty return code */
#define TASK_ENABLED 0 /**< The task enabled return code */
#define TASK_DISABLED 1 /**< The task disabled return code */
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

int ReadTask(task *t, char *storage_name, void *data); /**< Read task data */
int WriteTask(task *t, char *storage_name, void *data); /**< Write data to the task */

task* M2TaskLoad(module *m, pool *p, int tid);
int GetNewTask(module *m, pool *p, task *t, short ****board_buffer);
int M2TaskPrepare(module *m, pool *p, task *t);
int M2TaskProcess(module *m, pool *p, task *t);
int TaskRestore(module *m, pool *p, task *t);
void TaskReset(module *m, pool *p, task *t, int tid);
void TaskFinalize(module *m, pool *p, task *t);

#endif

