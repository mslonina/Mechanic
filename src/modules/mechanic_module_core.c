/**
 * @file
 * Core Mechanic module, implementation of all API functions
 */

#include "MMechanic2.h"
#include "mechanic_module_core.h"

/**
 * @function
 * Initializes critical core variables 
 *
 * At least the core module implements this function. The user may override the core
 * defaults in custom module -- in such case, the module variables will be merged with
 * core.
 */
int Init(init *i) {
  i->options = 128;
  i->pools = 64;
  i->banks_per_pool = 8;
  i->banks_per_task = 8;
  
  return TASK_SUCCESS;
}

/**
 * @function
 * Define the default configuration options
 *
 * This is an implementation of the LRC API. We initialize here default options.
 * If the user-supplied module implements this function, the setup options will be merged
 * with the core. All options will be available in the setup->head linked list.
 *
 * Configuration options are automatically added to the Popt arg table, so that, you may
 * override the defaults with the command line.
 */
int Setup(setup *s) {
  s->options[0] = (LRC_configDefaults) {
    .space="core", 
    .name="name", 
    .shortName='n', 
    .value="mechanic", 
    .type=LRC_STRING,
    .description="The name of the run"
  };
  s->options[1] = (LRC_configDefaults) {
    .space="core", 
    .name="xres", 
    .value="5", 
    .type=LRC_INT,
    .description="The task pool board dim[1] resolution"
  };
  s->options[2] = (LRC_configDefaults) {
    .space="core", 
    .name="yres", 
    .value="5", 
    .type=LRC_INT,
    .description="The task pool board dim[0] resolution"
  };
  s->options[3] = (LRC_configDefaults) {
    .space="core",
    .name="checkpoint",
    .value="2",
    .type=LRC_INT,
    .description="The checkpoint size (value is multiplied by mpi_size - 1)"
  };
  s->options[4] = (LRC_configDefaults) {LRC_OPTIONS_END};
  
  return TASK_SUCCESS;
}

/**
 * @function
 * Define the pool storage layout
 *
 * If the user-supplied module implements this function, the layout will be merged with
 * the core. You may change the layout per pool.
 *
 * POOL STORAGE
 *
 * The Pool will store its global data in the /Pools/pool-ID. The task data will be stored
 * in /Pools/pool-ID/Tasks. 
 *
 * The size of the storage dataset and the memory is the same. In the case of pool, whole
 * memory block is stored at once, if use_hdf = 1. 
 * 
 * The Pool data is broadcasted right after PoolPrepare() and saved to the master
 * datafile.
 *
 * TASK STORAGE
 * 
 * The task data is stored inside /Pools/pool-ID/Tasks group. There are three available
 * methods to store the task result:
 *
 * - STORAGE_BASIC - the whole memory block is stored in a dataset inside /Tasks/task-ID
 *   group
 * - STORAGE_PM3D - the memory block is stored in a dataset with a column-offset, so that
 *   the output is suitable to process with Gnuplot
 * - STORAGE_BOARD - the memory block is stored in a dataset with a {row,column}-offset
 *   according to the board-location of the task
 *
 * CHECKPOINT
 *
 * The checkpoint is defined as a multiply of the MPI_COMM_WORLD-1, minimum = 1. It
 * contains the results from tasks that have been processed and received. When the
 * checkpoint is full, the result is stored in the master datafile, according to the
 * storage information provided in the module.
 *
 * You can adjust the number of available memory/storage banks by implementing the Init
 * function and using banks_per_pool and banks_per_task variables.
 */
int Storage(pool *p, setup *s) {

  /* Path: /Pools/pool-ID/board */
  p->board->layout.path = "board";
  p->board->layout.rank = 2; // pool rank
  p->board->layout.dataspace_type = H5S_SIMPLE;
  p->board->layout.datatype = H5T_NATIVE_DOUBLE;
  p->board->layout.dim[0] = LRC_option2int("core", "xres", s->head); // vertical res
  p->board->layout.dim[1] = LRC_option2int("core", "yres", s->head); // horizontal res
  p->board->layout.use_hdf = 1;

  p->checkpoint_size = LRC_option2int("core", "checkpoint", s->head);

  return TASK_SUCCESS;
}

/**
 * @function
 * Prepares the pool
 *
 * This is a perfect place to read additional configuration, input files and assign values
 * to pool data tables. The data stored in the pool is broadcasted to all nodes, right
 * after the function is performed, as well as hdf storage for all datasets with use_hdf =
 * 1.
 */
int PoolPrepare(pool **allpools, pool *current, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * Process the pool
 *
 * @return
 * POOL_FINALIZE for the last pool or POOL_CREATE_NEW, if the pool loop have to continue
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  return POOL_FINALIZE;
}

/** 
 * @function
 * Maps tasks
 *
 * The mapping starts from the top left corner:
 *
 * (0,0) (0,1) (0,2) (0,3) ...
 * (1,0) (1,1) (1,2) (1,3)
 * (2,0) (2,1) (2,2) (2,3)
 *  ...
 *
 *  This follows the hdf5 storage, row by row:
 *
 *  0  1  2  3
 *  4  5  6  7
 *  8  9 10 11
 */
int TaskMapping(pool *p, task *t, setup *s) {
  int px, vert;

  px = t->tid;
  vert = p->board->layout.dim[1];

  if (px < vert) {
    t->location[0] = px / vert;
    t->location[1] = px;
  }
  if (px > vert - 1) {
    t->location[0] = px / vert;
    t->location[1] = px % vert;
  }

  return TASK_SUCCESS;
}

/**
 * @function
 * Prepares the task
 */
int TaskPrepare(pool *p, task *t, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * Process the task
 */
int TaskProcess(pool *p, task *t, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * Prepares the checkpoint
 *
 * @in_group
 * The master node
 */
int CheckpointPrepare(pool *p, checkpoint *c, setup *s) {
  return TASK_SUCCESS;
}
