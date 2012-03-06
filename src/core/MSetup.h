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

int Setup(module *m, char *filename, int mode);
int ReadConfig(char *filename, LRC_configNamespace *head);
int Popt(int node, int argc, char** argv, setup *s);

int PoptCountOptions(struct poptOption *p);
int PoptMergeOptions(module *m, struct poptOption *in, struct poptOption *add);
#endif
