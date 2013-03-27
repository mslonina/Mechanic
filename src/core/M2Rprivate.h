/**
 * @file
 * The restart mode
 */
#ifndef MECHANIC_M2R_PRIVATE_H
#define MECHANIC_M2R_PRIVATE_H

#include "M2Aprivate.h"
#include "M2Cprivate.h"
#include "M2Sprivate.h"
#include "M2Rpublic.h"

int Restart(module *m, pool **all, unsigned int *pool_counter);
int Validate(module *m, char *filename);

#endif
