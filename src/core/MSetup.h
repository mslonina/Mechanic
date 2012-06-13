/**
 * @file
 * The setup interface header
 */
#ifndef MECHANIC_SETUP_H
#define MECHANIC_SETUP_H

#include "MMechanic2.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MLog.h"
#include "MMpi.h"

#define SEPARATOR "=" /**< The variable/value separator */
#define COMMENTS "#" /**< The comment marker */
#define CONFIG_GROUP "all" /**< The configuration group in the master data file */
#define CORE_SETUP 0 /**< The core setup phase */
#define MODULE_SETUP 1 /**< The module setup phase */

int Setup(module *m, char *filename, int argc, char **argv, int setup_mode);
int ReadConfig(module *m, char *filename, LRC_configNamespace *head, int setup_mode);
int Popt(module *m, int argc, char** argv, setup *s, int setup_mode);
int PoptOptions(module *m, setup *s);
int LRCUpdate(setup *s);

#endif
