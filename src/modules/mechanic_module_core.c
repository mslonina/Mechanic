/**
 * @file
 * The core Mechanic module, implementation of all API functions
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

#include "mechanic.h"
#include "mechanic_module_core.h"

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
 * the core defaults.
 *
 * @ingroup all_nodes
 *
 * @param i The init structure
 * @return SUCCESS on success, error code otherwise
 */
int Init(init *i) {
  i->options = 128; /**< Maximum number of configurations option for the module */
  i->pools = 64; /**< Maximum number of task pools */
  i->banks_per_pool = 8; /**< Maximum number of memory bank per pool */
  i->banks_per_task = 8; /**< Maximum number of memory banks per task */
  i->attr_per_dataset = 128; /**< Maximum number of attributes that may be assigned to the dataset */
  i->min_cpu_required = 2; /**< Minimum number of CPUs required */

  return SUCCESS;
}

/**
 * @brief Define the configuration options
 *
 * This function is used to define configuration options. 
 * Configuration options are automatically available in the command line.
 *
 * If the Setup() hook is present in a custom module, the options are merged with the core
 * options. It is not possible to override core defaults from this hook. The only way to
 * do this is to use command line or configuration file.
 *
 * @ingroup all_nodes
 *
 * @param s The setup structure
 *
 * @return SUCCESS on success, error code otherwise
 */
int Setup(setup *s) {
  s->options[ 0] = (options) {
    .space="core", .name="name", .shortName='n', .value="mechanic", .type=C_STRING,
    .description="The name of the run (master filename prefix)"
  };
  s->options[ 1] = (options) {
    .space="core", .name="title", .shortName='\0', .value="", .type=C_STRING,
    .description="The title of the run"
  };
  s->options[ 2] = (options) {
    .space="core", .name="description", .shortName='\0', .value="", .type=C_STRING,
    .description="The short description of the run"
  };
  s->options[ 3] = (options) {
    .space="core", .name="module", .shortName='p', .value="core", .type=C_STRING,
    .description="The user-supplied module name"
  };
  s->options[ 4] = (options) {
    .space="core", .name="config", .shortName='c', .value="mechanic-config.cfg", .type=C_STRING,
    .description="The configuration file"
  };
  s->options[ 5] = (options) {
    .space="core", .name="xelement", .shortName='\0', .value="0", .type=C_INT,
    .description="The x-axis element"
  };
  s->options[ 6] = (options) {
    .space="core", .name="yelement", .shortName='\0', .value="0", .type=C_INT,
    .description="The y-axis element"
  };
  s->options[ 7] = (options) {
    .space="core", .name="zelement", .shortName='\0', .value="0", .type=C_INT,
    .description="The z-axis element"
  };
  s->options[ 8] = (options) {
    .space="core", .name="xlabel", .shortName='\0', .value="", .type=C_STRING,
    .description="The x-axis label"
  };
  s->options[ 9] = (options) {
    .space="core", .name="ylabel", .shortName='\0', .value="", .type=C_STRING,
    .description="The y-axis label"
  };
  s->options[10] = (options) {
    .space="core", .name="zlabel", .shortName='\0', .value="", .type=C_STRING,
    .description="The z-axis label"
  };
  s->options[11] = (options) {
    .space="core", .name="xorigin", .shortName='\0', .value="0.5", .type=C_DOUBLE,
    .description="The x-axis origin"
  };
  s->options[12] = (options) {
    .space="core", .name="yorigin", .shortName='\0', .value="0.5", .type=C_DOUBLE,
    .description="The y-axis origin"
  };
  s->options[13] = (options) {
    .space="core", .name="zorigin", .shortName='\0', .value="0.5", .type=C_DOUBLE,
    .description="The z-axis origin"
  };
  s->options[14] = (options) {
    .space="core", .name="xres", .shortName='x', .value="5", .type=C_INT,
    .description="The task pool board horizontal resolution"
  };
  s->options[15] = (options) {
    .space="core", .name="yres", .shortName='y', .value="5", .type=C_INT,
    .description="The task pool board vertical resolution"
  };
  s->options[16] = (options) {
    .space="core", .name="zres", .shortName='z', .value="1", .type=C_INT,
    .description="The task pool board depth resolution"
  };
  s->options[17] = (options) {
    .space="core", .name="xmin", .shortName='\0', .value="0.0", .type=C_DOUBLE,
    .description="The x-axis minimum"
  };
  s->options[18] = (options) {
    .space="core", .name="xmax", .shortName='\0', .value="1.0", .type=C_DOUBLE,
    .description="The x-axis maximum"
  };
  s->options[19] = (options) {
    .space="core", .name="ymin", .shortName='\0', .value="0.0", .type=C_DOUBLE,
    .description="The y-axis minimum"
  };
  s->options[20] = (options) {
    .space="core", .name="ymax", .shortName='\0', .value="1.0", .type=C_DOUBLE,
    .description="The y-axis maximum"
  };
  s->options[21] = (options) {
    .space="core", .name="zmin", .shortName='\0', .value="0.0", .type=C_DOUBLE,
    .description="The z-axis minimum"
  };
  s->options[22] = (options) {
    .space="core", .name="zmax", .shortName='\0', .value="1.0", .type=C_DOUBLE,
    .description="The z-axis maximum"
  };
  s->options[23] = (options) {
    .space="core", .name="checkpoint", .shortName='d', .value="2048", .type=C_INT,
    .description="The checkpoint size (number of tasks)"
  };
  s->options[24] = (options) {
    .space="core", .name="checkpoint-files", .shortName='b', .value="2", .type=C_INT,
    .description="The number of incremental backups of the master file"
  };
  s->options[25] = (options) {
    .space="core", .name="no-backup", .shortName='\0', .value="0", .type=C_VAL,
    .description="Disable the initial master file backup"
  };
  s->options[26] = (options) {
    .space="core", .name="restart-mode", .shortName='r', .value="0", .type=C_VAL,
    .description="The restart mode"
  };
  s->options[27] = (options) {
    .space="core", .name="restart-file", .shortName='\0', .value="restart-file.h5", .type=C_STRING,
    .description="The name of the file to use in the restart mode"
  };
  s->options[28] = (options) {
    .space="core", .name="blocking", .shortName='\0', .value="0", .type=C_VAL,
    .description="Switch to the blocking communication mode (for modules)"
  };
  s->options[29] = (options) {
    .space="core", .name="test", .shortName='\0', .value="0", .type=C_VAL,
    .description="Switch to test mode (for modules)"
  };
  s->options[30] = (options) {
    .space="core", .name="yes", .shortName='\0', .value="0", .type=C_VAL,
    .description="Skip checks (for modules)"
  };
  s->options[31] = (options) {
    .space="core", .name="dense", .shortName='\0', .value="0", .type=C_VAL,
    .description="Use dense output (for modules)"
  };
  s->options[32] = (options) {
    .space="core", .name="debug", .shortName='\0', .value="0", .type=C_VAL,
    .description="Switch to debug mode (for modules)"
  };
  s->options[33] = (options) {
    .space="core", .name="print-defaults", .shortName='\0', .value="0", .type=C_VAL,
    .description="Print default settings"
  };
  s->options[34] = (options) {
    .space="core", .name="help", .shortName='?', .value="0", .type=C_VAL,
    .description="Show this help message"
  };
  s->options[35] = (options) {
    .space="core", .name="usage", .shortName='\0', .value="0", .type=C_VAL,
    .description="Display brief message"
  };
  s->options[36] = (options) OPTIONS_END;

  return SUCCESS;
}

/**
 * @brief Define the task pool storage layout
 *
 * This function is used to define the storage layout. The layout may be defined per pool
 * (different storage layout during different task pools).
 *
 * If the Storage() hook is present in a custom module, the storage layout will be
 * merged with the core storage layout.
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
    .name = "board",
    .rank = TASK_BOARD_RANK+1, // pool rank
    .dims[0] = Option2Int("core", "yres", s->head), // vertical res
    .dims[1] = Option2Int("core", "xres", s->head), // horizontal res
    .dims[2] = Option2Int("core", "zres", s->head), // depth res
    .dims[3] = 3, // task status, computing node, task checkpoint number
    .datatype = H5T_NATIVE_SHORT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->checkpoint_size = Option2Int("core", "checkpoint", s->head);

  if (p->checkpoint_size <= 0) {
    p->checkpoint_size = 2048;
    ModifyOption("core", "checkpoint", "2048", C_INT, s->head);
  }

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
 *
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
 * is available at p->board->layout.dims array. The pool_size is a multiplication of
 * p->board->layout.dims[i], where i < p->board->layout.rank.
 *
 * This function is called during the TaskPrepare() phase.
 *
 * If the TaskBoardMap() hook is present in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup master_only
 * @param p The current pool structure
 * @param t The current task structure
 * @param s The setup structure
 *
 * @return SUCCESS or error code otherwise
 */
int TaskBoardMap(pool *p, task *t, setup *s) {
  int px, vert, horiz;

  px = t->tid;
  horiz = p->board->layout.dims[0];
  vert = p->board->layout.dims[1];

  t->location[2] = px / (vert * horiz);

  // Shift the current px on the 2D board
  if (t->location[2] > 0) {
    px = px - t->location[2] * (vert * horiz);
  }

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
  return TASK_FINALIZE;
}

/**
 * @brief Prepare the checkpoint
 * @todo Needs updating
 *
 * This function is used to prepare the checkpoint. You may do some data-related
 * operations. The data for the current checkpoint may be accessed by Read/WriteData()
 * through the current checkpoint memory pointer:
 *
 *     c->storage->memory
 *
 * which is a one-dimensional, flattened array, filled with c->size tasks, in a form:
 *
 *    header | task 0 datasets | header | task 1 datasets | ...
 *
 * The header contains HEADER_SIZE integer elements:
 *  - the MPI message tag
 *  - the received task ID
 *  - the received task status (TASK_FINISHED)
 *  - the received task location (TASK_BOARD_RANK, currently 2)
 *
 * It is best to keep this hook untouched, since the memory banks are used then to
 * physically store the data in the HDF5 master datafile. This hook should not be normally
 * used, unless you know what you are doing. You have been warned.
 *
 * If this hook is present in a custom module, it will be used instead the core one.
 *
 * @ingroup master_only
 * @param p The current pool structure
 * @param c The current checkpoint structrue
 * @param s The setup structure
 *
 * @return SUCCESS or error code otherwise
 */
int CheckpointPrepare(pool *p, checkpoint *c, setup *s) {
  Message(MESSAGE_INFO, "Checkpoint %04d processed\n", c->cid);
  return SUCCESS;
}

/**
 * @brief The prepare hook
 *
 * This function is used to do any simulation-related prepare operations. It is invoked
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
 * manipulation in the master data file. It is invoked after the pool loop is finished.
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
 * as well as top level group/file pointer. It is invoked right after the dataset is
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
 * as well as top level group/file pointer. It is invoked during PoolProcess(), 
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

/**
 * @brief The node prepare hook
 *
 * This is an advanced hook. It allows to perform additional stuff, such as direct memory
 * management or MPI communication. 
 *
 * This hook is invoked before the PoolReset() and PoolPrepare(), after the Setup(),
 * Prepare() and Storage() hooks.
 *
 * If the NodePrepare() hook is used in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup all_nodes
 * @param mpi_size The MPI_COMM_WORLD size
 * @param node The current node id
 * @param all The pointer to the all pools array
 * @param p The current pool pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */
int NodePrepare(int mpi_size, int node, pool **all, pool *p, setup *s) {
  return SUCCESS;
} 

/**
 * @brief The node process hook
 *
 * This is an advanced hook. It allows to perform additional stuff, such as direct memory
 * management or MPI communication.
 *
 * This hook is invoked after the PoolProcess().
 *
 * If the NodeProcess() hook is used in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup all_nodes
 * @param mpi_size The MPI_COMM_WORLD size
 * @param node The current node id
 * @param all The pointer to the all pools array
 * @param p The current pool pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */
int NodeProcess(int mpi_size, int node, pool **all, pool *p, setup *s) {
  return SUCCESS;
} 

/**
 * @brief The task loop prepare hook
 *
 * This is an advanced hook. It allows to perform additional stuff, such as direct memory
 * management or MPI communication. 
 *
 * This hook is invoked after the PoolReset() and PoolPrepare(), before entering the task
 * loop.
 *
 * If the LoopProcess() hook is used in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup all_nodes
 * @param mpi_size The MPI_COMM_WORLD size
 * @param node The current node id
 * @param all The pointer to the all pools array
 * @param p The current pool pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */
int LoopPrepare(int mpi_size, int node, pool **all, pool *p, setup *s) {
  return SUCCESS;
} 

/**
 * @brief The task loop process hook
 *
 * This is an advanced hook. It allows to perform additional stuff, such as direct memory
 * management or MPI communication.
 *
 * This hook is invoked after the task loop is completed, before the PoolProcess().
 *
 * If the LoopProcess() hook is used in a custom module, it will be used instead of the
 * core hook.
 *
 * @ingroup all_nodes
 * @param mpi_size The MPI_COMM_WORLD size
 * @param node The current node id
 * @param all The pointer to the all pools array
 * @param p The current pool pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */
int LoopProcess(int mpi_size, int node, pool **all, pool *p, setup *s) {
  return SUCCESS;
} 

/**
 * @brief The MPI Send hook
 *
 * This hook is performed after MPI_Send.
 *
 * @ingroup all_nodes
 * @param mpi_size The MPI_COMM_WORLD size
 * @param node The current node
 * @param dest The destination node
 * @param tag The message tag
 * @param p The current pool pointer
 * @param s The setup pointer
 *
 * @return SUCCESS or error code otherwise
 */
int Send(int mpi_size, int node, int dest, int tag, pool *p, setup *s) {
  return SUCCESS;
}

/**
 * @brief The MPI Receive hook
 *
 * This hook is performed after MPI_Send.
 *
 * @ingroup all_nodes
 * @param mpi_size The MPI_COMM_WORLD size
 * @param node The current node
 * @param sender The sender node
 * @param tag The message tag
 * @param p The current pool pointer
 * @param s The setup pointer
 * @param buffer The raw data buffer received
 *
 * @return SUCCESS or error code otherwise
 */
int Receive(int mpi_size, int node, int sender, int tag, pool *p, setup *s, void *buffer) {
  if (node == MASTER) {
    if (tag == TAG_RESULT) {
      Message(MESSAGE_RESULT, "Completed %4d of %4d tasks\n", p->completed, p->pool_size);
    }
  }
  return SUCCESS;
}

/** @} */

