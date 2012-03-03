#ifndef MECHANIC_SETUP_H
#define MECHANIC_SETUP_H

#include "MMechanic2.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MLog.h"
#include "MMpi.h"

#define SEPARATOR "="
#define COMMENTS "#"

int Setup(int node, module *m, char *filename, int mode);
int ReadConfig(char *filename, LRC_configNamespace *head);

#endif
