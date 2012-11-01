#ifndef MECHANIC_MODE_TASKFARM_H
#define MECHANIC_MODE_TASKFARM_H

#include "Mechanic2.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MLog.h"
#include "MPool.h"
#include "MMpi.h"
#include "MCheckpoint.h"
#include "MTask.h"
#include "MCore.h"
#include "MRestart.h"

typedef struct {
  storage *storage;
} mpi_message;

int Taskfarm(module *m);
int Master(module *m, pool *p);
int Worker(module *m, pool *p);
int MasterBlocking(module *m, pool *p);
int WorkerBlocking(module *m, pool *p);

#endif
