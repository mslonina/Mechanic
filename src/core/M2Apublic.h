/**
 * @file
 * Datatypes and function prototypes (public API)
 */
#ifndef MECHANIC_M2A_PUBLIC_H
#define MECHANIC_M2A_PUBLIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <time.h>

#include <hdf5.h>

#include "M2Cpublic.h"
#include "M2Epublic.h"

#define POOLS_GROUP "Pools"
#define POOL_PATH "/Pools/pool-%04d"
#define LAST_GROUP "/Pools/last"
#define TASKS_GROUP "Tasks"
#define TASK_PATH "task-%04d"
#define CORE_MODULE "core"

#define NORMAL_MODE 600 /**< The normal operation mode */
#define RESTART_MODE 601 /**< The restart mode */

#define TASK_BOARD_RANK 3 /**< The minimum task board rank */
#define TASK_NO_LOCATION 0 /**< Task location defaults */
#define TASK_EMPTY -88 /**< The task empty return code */

#define TAG_TERMINATE 12763 /** The node terminate tag */

/* Data */
#define HEADER_SIZE 4+TASK_BOARD_RANK /**< The data header size */
#define HEADER_INIT {TAG_TERMINATE,0,TASK_EMPTY,TASK_NO_LOCATION,TASK_NO_LOCATION,TASK_NO_LOCATION,0}

/**
 * @struct init
 * Bootstrap initializations
 */
typedef struct {
  unsigned int options; /**< The maxium size of the Config options table */
  unsigned int pools; /**< The maximum size of the pools array */
  unsigned int banks_per_pool; /**< The maximum number of memory/storage banks per pool */
  unsigned int banks_per_task; /**< The maximum number of memory/storage banks per task */
  unsigned int attr_per_dataset; /**< The maximum number of attributes per dataset */
  int min_cpu_required; /**< The minimum number of CPUs required */
} init;

/**
 * @struct layer
 * Defines the layer
 */
typedef struct {
  void *handler; /**< The module handler */
  void *mode_handler; /**< The runtime mode handler */
  init init; /**< The init structure */
  setup setup; /**< The setup structure */
} layer;

/**
 * @struct module
 * Defines the module
 */
typedef struct {
  char *filename; /**< The filename */
  hid_t datafile; /**< The datafile HDF5 pointer */
  hid_t h5location; /**< The HDF5 location pointer */
  int node; /**< The node ID */
  int mpi_size; /**< The MPI_COMM_WORLD size */
  int mode; /**< The running mode */
  int communication_type; /**< MPI communication type */
  int test; /**< Test mode */
  int dense; /**< Dense output */
  int yes; /**< Confirmation mode */
  int debug; /**< Debug mode */
  int showtime; /**< Show detailed CPU time */
  int verbose; /**< Be verbose */
  int stats; /**< Enable detailed stats */
  layer layer; /**< The layer pointer */
  layer fallback; /**< The fallback layer pointer */
} module;

/**
 * @typedef query
 * Basic dynamic query type
 */
typedef int (query) ();

char* Name(char *prefix, char *name, char *suffix, char *extension);
int Copy(char *in, char *out);
MPI_Datatype GetMpiDatatype(hid_t h5type);
hid_t GetHDF5Datatype(int ctype);

#endif
