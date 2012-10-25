/**
 * @file
 * Core Mechanic module, implementation of all API functions
 *
 * @todo provide hook examples
 */

/**
 * @defgroup all_nodes Hooks called on all nodes
 * @{
 */

/** @} */

/**
 * @defgroup master_only Hooks called on the master node only
 * @{
 */

/** @} */

/**
 * @defgroup worker_only Hooks called on worker nodes only
 * @{
 */

/** @} */

#include "Mechanic2.h"
#include "mechanic_module_core.h"

/**
 * @page pool_loop The pool loop explained
 *
 * After the core and the module are bootstrapped, the Mechanic enters the four-step pool
 * loop:
 *
 * 1. PoolPrepare()
 *  All nodes enter the PoolPrepare(). Data of all previous pools is passed to this
 *  function, so that we may use them to prepare data for the current pool. The current pool
 *  data is broadcasted to all nodes and stored in the master datafile.
 *
 * 2. The Task Loop
 *  All nodes enter the task loop. The master node prepares the task during
 *  TaskPrepare(). Each worker receives a task, and calls the TaskProcess(). The task
 *  data, marked with use_hdf = 1 are received by the master node after the
 *  TaskProcess() is finished and stored during the CheckpointProcess().
 *
 * 3. The Checkpoint
 *  The CheckpointPrepare() function might be used to adjust the received data (2D array
 *  of packed data, each row is a separate task). The pool data might be adjusted here
 *  as well, the data is stored again during CheckpointProcess().
 *
 * 4. PoolProcess()
 *  After the task loop is finished, the PoolProcess() is used to decide whether to
 *  continue the pool loop or not. The data of all pools is passed to this function. 
 *
 * @page task_loop The task loop explained
 */

/**
 * @defgroup public_api The Public API
 * @{
 */

/**
 * @brief Initialize critical core variables
 *
 * This function is used to initialize critical core variables, such as number of
 * configuration options, maximum number of task pools and the number of memory banks per
 * task and per pool. These variables are used to allocate the memory during the
 * simulation.
 *
 * The core module must implement this function, so that some sensible defaults are set.
 *
 * If the Init() hook is present in a custom module, the variables will be merged with
 * the core defaults, i.e.:
 *
 *     int Init(init *i) {
 *       i->options = 128;
 *       i->pools = 12;
 *     }
 *
 * @ingroup all_nodes
 *
 * @param i The init structure
 * @return SUCCESS on success, error code otherwise
 */

int Init(init *i) {
  i->options = 128;
  i->pools = 64;
  i->banks_per_pool = 8;
  i->banks_per_task = 8;
  i->attr_per_dataset = 8;

  return SUCCESS;
}

/**
 * @brief Define the configuration options
 *
 * This function is used to define configuration options. The LRC user API is used here.
 * Configuration options are automatically available in the command line.
 *
 * If the Setup() hook is present in a custom module, the options are merged with the core
 * options. It is not possible to override core defaults from this hook. The only way to
 * do this is to use command line or configuration file.
 *
 * The configuration is stored in the master datafile. Only the master node reads the
 * configuration file and parser the command line. The configuration is broadcasted to all
 * worker nodes.
 *
 * To obtain all available options, try the --help or --usage flag:
 * mpirun -np 2 mechanic2 -p mymodule --help
 *
 * ### Defining options
 *
 * The LRC API allows any kind of C99 struct initialization to be used, i.e.:
 *
 *     s->options[0] = (LRC_configDefaults) {
 *       .space="NAMESPACE",
 *       .name="VARIABLE",
 *       .shortName="V",
 *       .value="DEFAULT_VALUE",
 *       .type=TYPE,
 *       .description="SHORT DESCRIPTION"
 *     };
 *
 * where
 *
 * - space - the name of the configuration namespace (char string)
 * - name - the name of the variable (char string)
 * - shortName - the short name of the variable (char string), used in command line args,
 *   may be '\0 (no short option)
 * - value - the default value (char string)
 * - description - description for the variable (char string), used in command line args
 * - type - the variable type:
 *   - LRC_INT - integer variable
 *   - LRC_FLOAT - float variable
 *   - LRC_DOUBLE - double variable
 *   - LRC_STRING - char variable
 *   - LRC_VAL - variable that only updates its value (i.e. boolean)
 *
 * The options table must finish with LRC_OPTIONS_END:
 *
 *     s->options[13] = (LRC_configDefaults) LRC_OPTIONS_END;
 *
 * ### The config file
 *
 * The configuration file may be used, in a sample form (only one configuration
 * file both for core and the module):
 *
 *     [core] # namespace defined through .space="NAMESPACE"
 *     name = arnoldweb
 *     xres = 2048 # p->board->layout.dim[1], horizontal dim
 *     yres = 2048 # p->board->layout.dim[0], vertical dim
 *     checkpoint = 1024 # number of finished task to store 
 *
 *     [arnold]
 *     step = 0.3
 *     tend = 2000.0
 *     xmin = 0.95
 *     xmax = 1.05
 *     ymin = 0.95
 *     ymax = 1.05
 *     driver = 2
 *     eps = 0.0
 *     epsmax = 0.1
 *     eps_interval = 0.02
 *
 * @ingroup all_nodes
 *
 * @param s The setup structure
 *
 * @return SUCCESS on success, error code otherwise
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
    .value="2048",
    .type=LRC_INT,
    .description="The checkpoint size"
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
    .name="blocking",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Switch to the blocking communication mode"
  };
  s->options[11] = (LRC_configDefaults) {
    .space="core",
    .name="xmin",
    .shortName='\0',
    .value="0.0",
    .type=LRC_DOUBLE,
    .description="The x-axis minimum"
  };
  s->options[12] = (LRC_configDefaults) {
    .space="core",
    .name="xmax",
    .shortName='\0',
    .value="1.0",
    .type=LRC_DOUBLE,
    .description="The x-axis maximum"
  };
  s->options[13] = (LRC_configDefaults) {
    .space="core",
    .name="ymin",
    .shortName='\0',
    .value="0.0",
    .type=LRC_DOUBLE,
    .description="The y-axis minimum"
  };
  s->options[14] = (LRC_configDefaults) {
    .space="core",
    .name="ymax",
    .shortName='\0',
    .value="1.0",
    .type=LRC_DOUBLE,
    .description="The y-axis maximum"
  };
  s->options[15] = (LRC_configDefaults) {
    .space="core",
    .name="xorigin",
    .shortName='\0',
    .value="0.5",
    .type=LRC_DOUBLE,
    .description="The x-axis origin"
  };
  s->options[16] = (LRC_configDefaults) {
    .space="core",
    .name="yorigin",
    .shortName='\0',
    .value="0.5",
    .type=LRC_DOUBLE,
    .description="The y-axis origin"
  };
  s->options[17] = (LRC_configDefaults) {
    .space="core",
    .name="print-defaults",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Print default settings"
  };
  s->options[18] = (LRC_configDefaults) {
    .space="core",
    .name="help",
    .shortName='?',
    .value="0",
    .type=LRC_VAL,
    .description="Show this help message"
  };
  s->options[19] = (LRC_configDefaults) {
    .space="core",
    .name="usage",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Display brief message"
  };
  s->options[20] = (LRC_configDefaults) LRC_OPTIONS_END;

  return SUCCESS;
}

/**
 * @brief Define the pool storage layout
 *
 * This function is used to define the storage layout. The layout may be defined per pool
 * (different storage layout during different task pools).
 *
 * If the Storage() hook is present in a custom module, the storage layout will be
 * merged with the core storage layout.
 *
 * To define the dataset, any C99 struct initialization is allowed. i.e.:
 *
 *     p->storage[0].layout = (schema) {
 *       .path = "pool-data",
 *       .rank = 2,
 *       .dim[0] = 1, // horizontal size
 *       .dim[1] = 6, // vertical size
 *       .use_hdf = 1,
 *       .storage_type = STORAGE_GROUP,
 *       .sync = 1,
 *       .dataspace_type = H5S_SIMPLE,
 *       .datatype = H5T_NATIVE_DOUBLE,
 *     };
 *
 * where:
 * - path - the dataset name (char string)
 * - rank - the rank of the dataset (max 2)
 * - dim - the dimensions of the dataset
 * - use_hdf - whether to store dataset in the master file or not
 * - storage_type - the type of the storage to use (see below)
 * - sync - whether to broadcast the data to all computing pool
 * - dataspace_type - HDF5 dataspace type (for future development)
 * - datatype - HDF5 datatype (for future development)
 *
 * Note: The datatype is set to H5T_NATIVE_DOUBLE, the dataspace type is H5S_SIMPLE, and the max rank is 2.
 *
 * You can adjust the number of available memory/storage banks by implementing the Init()
 * hook (banks_per_pool, banks_per_task).
 *
 * ## The Pool storage
 *
 * The Pool will store its global data in the /Pools/pool-ID group, where the ID is the unique pool identifier.
 * The task data will be stored in /Pools/pool-ID/Tasks group.
 *
 * The size of the storage dataset and the memory is the same. In the case of pool, whole
 * memory block is stored at once, if use_hdf = 1.
 *
 * The Pool data is broadcasted right after the PoolPrepare() and saved to the master
 * datafile.
 *
 * Note: All global pool datasets must use STORAGE_GROUP storage_type.
 *
 * ## The task storage
 *
 * The task data is stored inside /Pools/pool-ID/Tasks group. The memory banks defined for
 * the task storage are synchronized between master and worker after the TaskProcess().
 *
 * There are four available methods to store the task result:
 *
 * ### STORAGE_GROUP
 *
 * The whole memory block is stored in a dataset inside /Tasks/task-ID
 * group (the ID is the unique task indentifier), i.e., for a dataset defined similar to:
 *
 *     p->task->storage[0].layout.path = "basic-dataset";
 *     p->task->storage[0].layout.dim[0] = 2;
 *     p->task->storage[0].layout.dim[1] = 6;
 *     p->task->storage[0].layout.storage_type = STORAGE_GROUP;
 *     p->task->storage[0].layout.use_hdf = 1;
 *
 * The output is stored in /Pools/pool-ID/Tasks/task-ID/basic-dataset:
 *
 *     9 9 9 9 9 9
 *     9 9 9 9 9 9
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
 * The size of the final dataset is pool_size * dim[1].
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
 * The size of the final dataset is pool_size * dim[1].
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
 * The size of the final dataset is pool_dim[0] * taks_dim[0] x pool_dim[1] * task_dim[1].
 *
 * ## Accessing the data
 *
 * All data my be accessed directly (the storage index follows the definition of the
 * datasets):
 *
 * - p->task->storage[0].data[0][0] for task datasets with storage_type STORAGE_PM3D,
 *   STORAGE_LIST, STORAGE_BOARD
 * - p->tasks[TID]->storage[1].data[0][0] for task datasets with storage_type STORAGE_GROUP.
 *   The TID is the task ID
 * - p->storage[0].data[0][0] for pool datasets
 *
 *
 * ## Checkpoint
 *
 * The checkpoint contains the results from tasks that have been processed and received. When the
 * checkpoint is filled up, the result is stored in the master datafile, according to the
 * storage information provided in the module.
 *
 * @ingroup all_nodes
 * @param p The pool structure
 * @param s The setup structure
 *
 * @return SUCCESS on success, error code otherwise
 */
int Storage(pool *p, setup *s) {

  /* Path: /Pools/pool-ID/board */
  p->board->layout = (schema) {
    .path = "board",
    .rank = 2, // pool rank
    .dim[0] = LRC_option2int("core", "yres", s->head), // vertical res
    .dim[1] = LRC_option2int("core", "xres", s->head), // horizontal res
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->checkpoint_size = LRC_option2int("core", "checkpoint", s->head);

  p->storage[0].layout = (schema) {
    .path = "integer-datatype",
    .rank = 2,
    .dim[0] = 3,
    .dim[1] = 4,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->storage[1].layout = (schema) {
    .path = "double-datatype",
    .rank = 2,
    .dim[0] = 3,
    .dim[1] = 4,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->storage[2].layout = (schema) {
    .path = "float-datatype",
    .rank = 2,
    .dim[0] = 3,
    .dim[1] = 4,
    .datatype = H5T_NATIVE_FLOAT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->task->storage[0].layout = (schema) {
    .path = "double-datatype",
    .rank = 2,
    .dim[0] = 3,
    .dim[1] = 3,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  p->task->storage[1].layout = (schema) {
    .path = "integer-datatype",
    .rank = 2,
    .dim[0] = 3,
    .dim[1] = 3,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };



  return SUCCESS;
}

/**
 * @brief Prepare the task pool
 *
 * This function is used to prepare the task pool. All memory banks defined in Storage()
 * hook are available for usage. This hook is followed by an automatic broadcast of all
 * memory banks with flag sync = 1 and storage (when use_hdf = 1).
 *
 * Example usage:
 * - reading additional configuration, input files etc.
 * - assign default values, preparing global data etc.
 *
 * During the task pool loop, the data from all previous pools is available in allpools.
 *
 * If the PoolPrepare() hook is present in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup master_only
 * @param allpools The pointer to all pools
 * @param current The current pool structure
 * @param s The setup structure
 *
 * @return SUCCESS on success, error code otherwise
 */
int PoolPrepare(pool **allpools, pool *current, setup *s) {
  int data[6][8];
  double buff[6][8];
  float floa[6][8];
  int i,j;

  for (i = 0; i < 6; i++) {
    for (j = 0; j < 8; j++) {
      data[i][j] = i + j;
      buff[i][j] = 47.13 + i + j;
      floa[i][j] = 80.23 + i + j;
    }
  }

  SetData(&(current->storage[0]), data);
  SetData(&(current->storage[1]), buff);
  SetData(&(current->storage[2]), floa);

  return SUCCESS;
}

/**
 * @brief Process the pool
 *
 * This function is used to process the task pool after the task loop is finished and
 * decide whether to continue the pool loop or finish the overall simulation. The data of
 * current pool as well as all previous pools is available.
 *
 * If the PoolProcess() hook is present in a custom module, it will be used instead of the
 * core hook.
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
 * ### Example
 *
 *     int PoolProcess(pool **allpools, pool *current, setup *s) {
 *       if (current->rid < 5) {
 *         printf("Pool reset ID: %d\n", current->rid);
 *         return POOL_RESET;
 *       }
 *       printf("Pool finalized after %d resets\n", current->rid);
 *       return POOL_FINALIZE;
 *     }
 *
 * @ingroup master_only
 * @param allpools The pointer to all pools
 * @param current The current pool structure
 * @param s The setup structure
 *
 * @return POOL_FINALIZE, POOL_CREATE_NEW, POOL_RESET
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  return POOL_FINALIZE;
}

/**
 * @brief Maps tasks
 *
 * The default task mapping follows the HDF5 storage scheme.
 * The mapping starts from the top left corner:
 *
 *           -- dim[1] --
 *     (0,0) (0,1) (0,2) (0,3) ...
 *     (1,0) (1,1) (1,2) (1,3)     | dim[0]
 *     (2,0) (2,1) (2,2) (2,3)
 *     ...
 *
 *  which will result, row by row:
 *
 *      0  1  2  3
 *      4  5  6  7
 *      8  9 10 11
 *
 * The current task location is available at t->location array. The pool resolution
 * is available at p->board->layout.dim array. The pool_size is a multiplication of
 * p->board->layout.dim[i], where i < p->board->layout.rank.
 *
 * This function is called during the TaskPrepare() phase.
 *
 * If the TaskMapping() hook is present in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup master_only
 * @param p The current pool structure
 * @param t The current task structure
 * @param s The setup structure
 *
 * @return SUCCESS or error code otherwise
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

  return SUCCESS;
}

/**
 * @brief Prepare the task
 *
 * This function is used to do any task-specific preparation, i.e. changing initial
 * conditions according to the current task location. The task ID and task location, as
 * well as global pool data and setup is available, i.e.:
 *
 * - t->tid - the ID of the current task
 * - t->location[0] - the horizontal position of the current task
 * - t->location[1] - the vertical position of the current task
 *
 * If the TaskPrepare() is present in a custom module, it will be used instead of the core
 * hook.
 *
 * @ingroup worker_only
 * @param p The current pool structure
 * @param t The current task structure
 * @param s The setup structure
 *
 * @return SUCCESS on success of error code otherwise
 */
int TaskPrepare(pool *p, task *t, setup *s) {
  return SUCCESS;
}

/**
 * @brief Process the task
 *
 * This function is used to process the current task, i.e. to perform main computations.
 * The data for the current task may be prepared in the TaskPrepare() hook.
 *
 * If the TaskProcess() is present in a custom module, it will be used instead of the core
 * hook.
 *
 * @ingroup worker_only
 * @param p The current pool structure
 * @param t The current task structure
 * @param s The setup structure
 *
 * @return SUCCESS on success or error code otherwise
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double data[3][3];
  int idata[3][3];
  int i,j;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      data[i][j] = t->tid+1.1;
      idata[i][j] = t->tid+1;
    }
  }
  SetData(&t->storage[0], data);
  SetData(&t->storage[1], idata);
  return SUCCESS;
}

/**
 * @brief Prepare the checkpoint
 *
 * This function is used to prepare the checkpoint. You may do some data-related
 * operations. The data for the current checkpoint may be accessed through the current
 * 2D checkpoint pointer:
 *
 *     c->storage->data[][]
 *
 * where the first index is the checkpoint memory bank (0 to checkpoint_size) and
 * the second index contains one-dimensional (flat) data from the received task (all
 * task memory banks packed into one array), i.e.
 *
 *    c->storage->data[0][0] - the MPI message tag
 *    c->storage->data[0][1] - the received task ID
 *    c->storage->data[0][2] - the received task status (TASK_FINISHED)
 *    c->storage->data[0][3] - the received task location[0]
 *    c->storage->data[0][4] - the received task location[1]
 *    c->storage->data[0][5] - the received task data begins here
 *
 * This hook should not be normally used, if it is present in a custom module, it will be
 * used instead the core hook.
 *
 * @ingroup master_only
 * @param p The current pool structure
 * @param c The current checkpoint structrue
 * @param s The setup structure
 *
 * @return SUCCESS or error code otherwise
 */
int CheckpointPrepare(pool *p, checkpoint *c, setup *s) {
  Message(MESSAGE_COMMENT, "Checkpoint %04d processed\n", c->cid);
  return SUCCESS;
}

/**
 * @brief The prepare hook
 *
 * This function is used to do any simulation-related prepare operations. It is called
 * before the pool loop starts.
 *
 * If the Prepare() hook is present in a custom module, it will be used instead of the core
 * hook.
 *
 * @ingroup all_nodes
 * @param node The node id
 * @param masterfile The name of the master data file
 * @param s The setup structure
 *
 * @return SUCCESS or error code otherwise
 */
int Prepare(int node, char *masterfile, setup *s) {
  return SUCCESS;
}

/**
 * @brief The process hook
 *
 * This function is used to perform any simulation post operations, such as specific data
 * manipulation in the master data file. It is called after the pool loop is finished.
 *
 * If the Process() hook is present in a custom module, it will be used instead of the core
 * hook.
 *
 * @ingroup all_nodes
 * @param node The node id
 * @param masterfile The name of the master data file
 * @param all The pointer to all pools 
 * @param s The setup structure
 *
 * @return SUCCESS or error code otherwise
 */
int Process(int node, char *masterfile, pool **all, setup *s) {
  return SUCCESS;
}

/**
 * @brief The dataset prepare hook
 *
 * This function may be used to prepare given dataset. The HDF5 dataset pointer is passed,
 * as well as top level group/file pointer. It is called right after the dataset is
 * created in the master data file, only one the master node. 
 *
 * As an example, you may write any initial data to the dataset, as well as different
 * attributes.
 *
 * If the DatasetPrepare() hook is used in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup master_only
 * @param h5location The top level location pointer according to the current dataset
 * @param h5dataset The dataset pointer
 * @param p The current pool pointer
 * @param d The current dataset storage pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */
int DatasetPrepare(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s) {
  return SUCCESS;
}

/**
 * @brief The dataset process hook
 *
 * This function may be used to process given dataset. The HDF5 dataset pointer is passed,
 * as well as top level group/file pointer. It is called during PoolProcess(), 
 * only one the master node. 
 *
 * As an example, you may process any data in the dataset, as well as different
 * attributes.
 *
 * If the DatasetProcess() hook is used in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup master_only
 * @param h5location The top level location pointer according to the current dataset
 * @param h5dataset The dataset pointer
 * @param p The current pool pointer
 * @param d The current dataset storage pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */

int DatasetProcess(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s) {
  return SUCCESS;
}
/** @} */
