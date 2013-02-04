/**
 * @file
 * The setup interface header
 */
#ifndef MECHANIC_M2CS_PRIVATE_H
#define MECHANIC_M2CS_PRIVATE_H

#include "M2Apublic.h"
#include "M2Cpublic.h"
#include "M2Cprivate.h"
#include "M2CSpublic.h"
#include "M2Mpublic.h"
#include "M2Spublic.h"

#define SEPARATOR "=" /**< The variable/value separator */
#define COMMENTS "#" /**< The comment marker */
#define CORE_SETUP 0 /**< The core setup phase */
#define MODULE_SETUP 1 /**< The module setup phase */

int Setup(module *m, char *filename, int argc, char **argv, int setup_mode);
int ReadConfig(module *m, char *filename, configNamespace *head, int setup_mode);
int Popt(module *m, int argc, char** argv, setup *s, int setup_mode);
int PoptOptions(module *m, setup *s);
int ConfigUpdate(setup *s);
int ConfigDatatype(options c, MPI_Datatype *mpi_t);

#endif
