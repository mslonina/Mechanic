/**
 * @file
 * The Mechanic public API
 */
#ifndef MECHANIC_MECHANIC_H
#define MECHANIC_MECHANIC_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <dlfcn.h>
#include <math.h>
#include <popt.h>
#include <mpi.h>
#include <hdf5.h>
#include <hdf5_hl.h>

/* Public headers */
#include "M2Error.h"
#include "M2Mpi.h"
#include "M2Storage.h"
#include "M2Setup.h"
#include "M2Task.h"
#include "M2Pool.h"

#define NORMAL_MODE 600 /**< The normal operation mode */
#define RESTART_MODE 601 /**< The restart mode */

/* Data */
#define HEADER_SIZE 4+TASK_BOARD_RANK /**< The data header size */
#define HEADER_INIT {TAG_TERMINATE,TASK_EMPTY,TASK_EMPTY,TASK_NO_LOCATION,TASK_NO_LOCATION,TASK_NO_LOCATION,0}

/**
 * @struct init
 * Bootstrap initializations
 */
typedef struct {
  int options; /**< The maxium size of the Config options table */
  int pools; /**< The maximum size of the pools array */
  int banks_per_pool; /**< The maximum number of memory/storage banks per pool */
  int banks_per_task; /**< The maximum number of memory/storage banks per task */
  int attr_per_dataset; /**< The maximum number of attributes per dataset */
  int min_cpu_required; /**< The minimum number of CPUs required */
} init;

/* Vendor */
#include "mechanic_rngs.h"

#endif

