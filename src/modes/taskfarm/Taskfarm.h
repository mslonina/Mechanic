/**
 * @file
 * The taskfarm mode
 */
#ifndef MECHANIC_MODE_TASKFARM_H
#define MECHANIC_MODE_TASKFARM_H

#include "mechanic.h"

int Master(module *m, pool *p);
int Worker(module *m, pool *p);

#endif
