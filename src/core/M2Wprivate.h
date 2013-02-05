#ifndef MECHANIC_M2W_PRIVATE_H
#define MECHANIC_M2W_PRIVATE_H

#include <time.h>

#include "M2Spublic.h"
#include "M2Tpublic.h"
#include "M2Ppublic.h"

#include "M2Pprivate.h"
#include "M2Rprivate.h"

#include "M2Wpublic.h"

int Work(module *m);
int M2Master(module *m, pool *p);
int M2Worker(module *m, pool *p);

#endif
