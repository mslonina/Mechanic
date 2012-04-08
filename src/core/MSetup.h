#ifndef MECHANIC_SETUP_H
#define MECHANIC_SETUP_H

#include "MMechanic2.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MLog.h"
#include "MMpi.h"

#define SEPARATOR "="
#define COMMENTS "#"
#define CONFIG_GROUP "all"

int Setup(module *m, char *filename, int argc, char **argv, int mode);
int ReadConfig(module *m, char *filename, LRC_configNamespace *head);
int Popt(module *m, int argc, char** argv, setup *s);
int PoptOptions(module *m, setup *s);
int LRCUpdate(setup *s);

#endif
