/**
 * @file
 */
#ifndef MECHANIC_TASK_H
#define MECHANIC_TASK_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"
#include "MStorage.h"

typedef struct {
  int tag;
  int pid;
  int tid;
  int location[MAX_RANK];
  double *data;
} mpi_task_t;

task TaskLoad(module *m, pool *p, int tid);
int TaskInit(module *m, pool *p, task *t);
int TaskPrepare(module *m, pool *p, task *t);
int TaskProcess(module *m, pool *p, task *t);
void TaskFinalize(module *m, pool *p, task *t);
void BuildTaskDerivedType(task *t, int banks, mpi_task_t *mt, MPI_Datatype *ptr);

#endif