#ifndef MECHANIC_MODE_TASKFARM_H
#define MECHANIC_MODE_TASKFARM_H

#include "MMechanic2.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MLog.h"
#include "MPool.h"

int Taskfarm(module *m);
int Master(module *m);
int Worker(module *m);

#endif
