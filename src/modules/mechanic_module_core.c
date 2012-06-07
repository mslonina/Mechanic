/**
 * @file
 * Core Mechanic module, implementation of all API functions
 */

/**
 * @defgroup public_api The Public API
 * @{
 */

#include "MMechanic2.h"
#include "mechanic_module_core.h"

int reset_counter = 0;

/**
 * @function
 * @brief Initializes critical core variables
 *
 * At least the core module implements this function. The user may override the core
 * defaults in a custom module -- in such a case, the module variables will be merged with
 * core.
 *
 * @param i The init structure
 * @return TASK_SUCCESS on success, error code otherwise
 */

/**
 * .. c:function:: int Init(init *i)
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
 * @brief Define the default configuration options
 *
 * This is an implementation of the LRC API. We initialize here default options.
 * If the user-supplied module implements this function, the setup options will be merged
 * with the core. All options will be available in the setup->head linked list.
 *
 * Configuration options are automatically added to the Popt arg table, so that, you may
 * override the defaults with the command line.
 *
 * @param s The setup structure
 * @return TASK_SUCCESS on success, error code otherwise
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
    .shortName='x',
    .value="5",
    .type=LRC_INT,
    .description="The task pool board horizontal resolution"
  };
  s->options[2] = (LRC_configDefaults) {
    .space="core",
    .name="yres",
    .shortName='y',
    .value="5",
    .type=LRC_INT,
    .description="The task pool board vertical resolution"
  };
  s->options[3] = (LRC_configDefaults) {
    .space="core",
    .name="checkpoint",
    .shortName='d',
    .value="2",
    .type=LRC_INT,
    .description="The checkpoint size (value is multiplied by mpi_size - 1)"
  };
  s->options[4] = (LRC_configDefaults) {
    .space="core",
    .name="module",
    .shortName='p',
    .value="core",
    .type=LRC_STRING,
    .description="The user-supplied module name"
  };
  s->options[5] = (LRC_configDefaults) {
    .space="core",
    .name="config",
    .shortName='c',
    .value="mechanic-config.cfg",
    .type=LRC_STRING,
    .description="The configuration file"
  };
  s->options[6] = (LRC_configDefaults) {
    .space="core",
    .name="checkpoint-files",
    .shortName='b',
    .value="2",
    .type=LRC_INT,
    .description="The number of incremental backups of the master file"
  };
  s->options[7] = (LRC_configDefaults) {
    .space="core",
    .name="no-backup",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Disable the initial master file backup"
  };

  s->options[8] = (LRC_configDefaults) {
    .space="core",
    .name="restart-mode",
    .shortName='r',
    .value="0",
    .type=LRC_VAL,
    .description="The restart mode"
  };
  s->options[9] = (LRC_configDefaults) {
    .space="core",
    .name="restart-file",
    .shortName='\0',
    .value="restart-file.h5",
    .type=LRC_STRING,
    .description="The name of the file to use in the restart mode"
  };
  s->options[10] = (LRC_configDefaults) {
    .space="core",
    .name="print-defaults",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Print default settings"
  };
  s->options[11] = (LRC_configDefaults) {
    .space="core",
    .name="help",
    .shortName='?',
    .value="0",
    .type=LRC_VAL,
    .description="Show this help message"
  };
  s->options[12] = (LRC_configDefaults) {
    .space="core",
    .name="usage",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Display brief message"
  };
  s->options[13] = (LRC_configDefaults) LRC_OPTIONS_END;

  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Define the pool storage layout
 *
 * If the user-supplied module implements this function, the layout will be merged with
 * the core. You may change the layout per pool.
 *
 * ## POOL STORAGE
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
 * Note: All global pool datasets must use STORAGE_BASIC storage_type.
 *
 * ## TASK STORAGE
 *
 * The task data is stored inside /Pools/pool-ID/Tasks group. There are four available
 * methods to store the task result:
 *
 * ### STORAGE_BASIC
 *
 * The whole memory block is stored in a dataset inside /Tasks/task-ID
 * group, i.e., for a dataset defined similar to:
 *
 *     p->task->storage[0].layout.path = "basic-dataset";
 *     p->task->storage[0].layout.dim[0] = 2;
 *     p->task->storage[0].layout.dim[1] = 6;
 *     p->task->storage[0].layout.storage_type = STORAGE_BASIC;
 *     p->task->storage[0].layout.use_hdf = 1;
 *
 * The output is stored in /Pools/pool-ID/Tasks/task-ID/basic-dataset:
 *
 *     9 9 9 9 9 9
 *     9 9 9 9 9 9
 *
 *
 * ### STORAGE_PM3D
 *
 * The memory block is stored in a dataset with a column-offset, so that
 * the output is suitable to process with Gnuplot. Example: Suppose we have a 2x5 task
 * pool:
 *
 *     1 2 3 4 5
 *     6 7 8 9 0
 *
 * While each worker returns the result of size 2x7. For a dataset defined similar to:
 *
 *     p->task->storage[0].layout.path = "pm3d-dataset";
 *     p->task->storage[0].layout.dim[0] = 2;
 *     p->task->storage[0].layout.dim[1] = 7;
 *     p->task->storage[0].layout.storage_type = STORAGE_PM3D;
 *     p->task->storage[0].layout.use_hdf = 1;
 *
 * We have: /Pools/pool-ID/Tasks/pm3d-dataset with:
 *
 *     1 1 1 1 1 1 1
 *     1 1 1 1 1 1 1
 *     6 6 6 6 6 6 6
 *     6 6 6 6 6 6 6
 *     2 2 2 2 2 2 2
 *     2 2 2 2 2 2 2
 *     7 7 7 7 7 7 7
 *     7 7 7 7 7 7 7
 *     ...
 *
 * ### STORAGE_LIST
 *
 * The memory block is stored in a dataset with a task-ID offset, This is
 * similar to STORAGE_PM3D, this time however, there is no column-offset. For a dataset
 * defined as below:
 *
 *     p->task->storage[0].layout.path = "list-dataset";
 *     p->task->storage[0].layout.dim[0] = 2;
 *     p->task->storage[0].layout.dim[1] = 7;
 *     p->task->storage[0].layout.storage_type = STORAGE_LIST;
 *     p->task->storage[0].layout.use_hdf = 1;
 *
 * The output is stored in /Pools/pool-ID/Tasks/list-dataset:
 *
 *     1 1 1 1 1 1 1
 *     1 1 1 1 1 1 1
 *     2 2 2 2 2 2 2
 *     2 2 2 2 2 2 2
 *     3 3 3 3 3 3 3
 *     3 3 3 3 3 3 3
 *     4 4 4 4 4 4 4
 *     4 4 4 4 4 4 4
 *     ...
 *
 * ### STORAGE_BOARD
 *
 * The memory block is stored in a dataset with a {row,column}-offset
 * according to the board-location of the task. Suppose we have a dataset defined like
 * this:
 *
 *     p->task->storage[0].layout.path = "board-dataset";
 *     p->task->storage[0].layout.dim[0] = 2;
 *     p->task->storage[0].layout.dim[1] = 3;
 *     p->task->storage[0].layout.storage_type = STORAGE_BOARD;
 *     p->task->storage[0].layout.use_hdf = 1;
 *
 * For a 2x5 task pool:
 *
 *     1 2 3 4 5
 *     6 7 8 9 0
 *
 * the result is stored in /Pools/pool-ID/Tasks/board-dataset:
 *
 *     1 1 1 2 2 2 3 3 3 4 4 4 5 5 5
 *     1 1 1 2 2 2 3 3 3 4 4 4 5 5 5
 *     6 6 6 7 7 7 8 8 8 9 9 9 0 0 0
 *     6 6 6 7 7 7 8 8 8 9 9 9 0 0 0
 *
 *
 * ## CHECKPOINT
 *
 * The checkpoint is defined as a multiply of the MPI_COMM_WORLD-1, minimum = 1. It
 * contains the results from tasks that have been processed and received. When the
 * checkpoint is full, the result is stored in the master datafile, according to the
 * storage information provided in the module.
 *
 * You can adjust the number of available memory/storage banks by implementing the Init
 * function and using banks_per_pool and banks_per_task variables.
 *
 * @param p The pool structure
 * @param s The setup structure
 *
 * @return TASK_SUCCESS on success, error code otherwise
 */
int Storage(pool *p, setup *s) {

  /* Path: /Pools/pool-ID/board */
  p->board->layout = (schema) {
    .path = "board",
    .rank = 2, // pool rank
    .dim[0] = LRC_option2int("core", "xres", s->head), // vertical res
    .dim[1] = LRC_option2int("core", "yres", s->head), // horizontal res
    .use_hdf = 1,
    .storage_type = STORAGE_BASIC,
  };

  p->checkpoint_size = LRC_option2int("core", "checkpoint", s->head);

  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Prepares the pool
 *
 * This is a perfect place to read additional configuration, input files and assign values
 * to pool data tables. The data stored in the pool is broadcasted to all nodes, right
 * after the function is performed, as well as hdf storage for all datasets with use_hdf =
 * 1
 *
 * @param allpools The pointer to all pools
 * @param p The current pool structure
 * @param s The setup structure
 *
 * @return TASK_SUCCESS on success, error code otherwise
 */
int PoolPrepare(pool **allpools, pool *current, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Process the pool
 *
 * @param allpools The pointer to all pools
 * @param p The current pool structure
 * @param s The setup structure
 *
 * ### Return values
 *
 * - POOL_CREATE_NEW - if the pool loop should continue (the new pool will be created)
 * - POOL_RESET - reset the current pool: the task board will be reset. The task loop can
 *   be restarted within the same loop, i.e. from sligthly different startup values
 *   Hybrid Genetic Algoriths example:
 *   1. Compute first iteration of children
 *   2. Loop N-times in the children loop using POOL_RESET (and p->rid counter), to
 *   improve the current generation
 *   3. Create new generation with POOL_CREATE_NEW
 *
 * - POOL_FINALIZE - finalizes the pool loop and simulation
 *
 * @return
 * POOL_FINALIZE, POOL_CREATE_NEW, POOL_RESET
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  if (current->rid < 5) {
    printf("Pool reset ID: %d\n", current->rid);
    return POOL_RESET;
  }
  printf("Pool finalized after %d resets\n", current->rid);
  return POOL_FINALIZE;
}

/**
 * @function
 * @brief Maps tasks
 *
 * The mapping starts from the top left corner:
 *
 *     (0,0) (0,1) (0,2) (0,3) ...
 *     (1,0) (1,1) (1,2) (1,3)
 *     (2,0) (2,1) (2,2) (2,3)
 *     ...
 *
 *  This follows the hdf5 storage, row by row:
 *
 *      0  1  2  3
 *      4  5  6  7
 *      8  9 10 11
 *
 *  @param p
 *  @param t
 *  @param s
 *
 *  @return
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
 * @brief Prepares the task
 *
 * @param p
 * @param t
 * @param s
 *
 * @return
 */
int TaskPrepare(pool *p, task *t, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Process the task
 *
 * @param p
 * @param t
 * @param s
 *
 * @return
 */
int TaskProcess(pool *p, task *t, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Prepares the checkpoint
 *
 * @param p
 * @param c
 * @param s
 *
 * @return
 *
 */
int CheckpointPrepare(pool *p, checkpoint *c, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * @brief The prepare hook
 *
 * This hook is called right before the pool starts
 */
int Prepare(char *masterfile, setup *s) {
  return TASK_SUCCESS;
}

/**
 * @function
 * @brief The process hook
 *
 * This hook is called after the pool loop finishes.
 * Example:
 * - You may do some specific data file postprocessing here
 */
int Process(char *masterfile, setup *s) {
  return TASK_SUCCESS;
}
/** @} */
