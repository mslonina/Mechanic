/**
 * Mechanic2 API proposal
 */

/**
 * What must be changed:
 *  - LRC datatypes and the library at all (@done: much less memory allocations)
 *  - module hook loading - allow using two modules, one by one
 *  - non-blocking communication inside task loop
 */

/**
 * [taskpool loop]
 *    taskpool_setup
 *    taskpool_init
 *    taskpool_storage
 *    taskpool_prepare
 *
 *    [task loop]
 *      task_prepare
 *      task_process
 *      task_storage_offsets
 *    [/task loop]
 *
 *    taskpool_postprocess
 *  [/taskpool loop]
 */

#define MAX_RANK 4

typedef struct {
  int node; // node id
  int MPISize; // MPI Comm size
  int pid; // pool id
  int rank; // pool rank
  int dimsf[MAX_RANK]; 
  int dimsm[MAX_RANK];
  int itmp[][]; // temporary integer memory banks
  double dtmp[][]; // temporary double memory banks
} Pool;

typedef struct {

} Setup;

typedef struct {
  char path[256]; // Path in HDF5 file
  int rank; // HDF5 dataset rank
  int dimsf[MAX_RANK]; // File dataset size
  int dimsm[MAX_RANK]; // Memory block for the dataset
  int hdf; // Whether to use HDF5 storage or not, 1 or 0
} Data;

typedef struct {
  int tid; // Task id
  int rank; // Task rank
  int coordinates[MAX_RANK]; // Task coordinates
} Task;

/**
 * @function
 * Defines custom module/pool setup with LRC-API style
 *
 * @in_group
 * Not required, hook performed right after prototype
 *
 * @in_group
 * All nodes
 *
 * @after
 * Mechanic reads own configuration and the module configuration. The configuration is
 * stored in master datafile and may be reused during restart.
 *
 * @possibility
 * Mechanic loads prototype with own configuration options, and then the module hook, if
 * any, so that we can use only one config file, i.e.
 * 
 *  allocate Setup (possibly some large value for module hook)
 *  module_taskpool_setup(&Setup)
 *  hello_taskpool_setup(&Setup) <- module adds some configuration in module LRC-namespace
 *
 * @deprecated
 * mconfig option shall not be used anymore
 */
int module_taskpool_setup(Setup *s) {

  /* Mechanic defaults */
  s->core.config[0] = (LRC_configDefaults) {"default", "name", MECHANIC_NAME_DEFAULT, LRC_STRING};
  s->core.config[1] = (LRC_configDefaults) {"default", "xres", MECHANIC_XRES_DEFAULT, LRC_INT};
  s->core.config[2] = (LRC_configDefaults) {"default", "yres", MECHANIC_YRES_DEFAULT, LRC_INT};
  s->core.config[3] = (LRC_configDefaults) {"default", "module", MECHANIC_MODULE_DEFAULT, LRC_STRING};
  s->core.config[4] = (LRC_configDefaults) {"default", "mconfig", MECHANIC_CONFIG_FILE_DEFAULT, LRC_STRING};
  s->core.config[5] = (LRC_configDefaults) {"default", "mode", MECHANIC_MODE_DEFAULT, LRC_INT};
  s->core.config[6] = (LRC_configDefaults) {"logs", "checkpoint", MECHANIC_CHECKPOINT_DEFAULT, LRC_INT};
  s->core.config[7] = (LRC_configDefaults) LRC_OPTIONS_END;

  return MECHANIC_TASK_SUCCESS;

}

int hello_taskpool_setup(Setup *s) {

  s->module.config[0] = (LRC_configDefaults) {"arnold", "step", "0.25", LRC_DOUBLE};
  s->module.config[1] = (LRC_configDefaults) {"arnold", "tend", "1000.0", LRC_DOUBLE};
  s->module.config[2] = (LRC_configDefaults) {"arnold", "xmin", "0.8", LRC_DOUBLE};
  s->module.config[3] = (LRC_configDefaults) {"arnold", "xmax", "1.2", LRC_DOUBLE};
  s->module.config[4] = (LRC_configDefaults) {"arnold", "ymin", "0.8", LRC_DOUBLE};
  s->module.config[5] = (LRC_configDefaults) {"arnold", "ymax", "1.2", LRC_DOUBLE};
  s->module.config[6] = (LRC_configDefaults) {"arnold", "eps", "0.01", LRC_DOUBLE};
  s->module.config[7] = (LRC_configDefaults) {"arnold", "driver", "1", LRC_INT};
  s->module.config[8] = (LRC_configDefaults) LRC_OPTIONS_END;
  
  return MECHANIC_TASK_SUCCESS;

}

/**
 * @function
 * Initializes pool and setups sane defaults
 *
 * Mechanic loads the prototype first, to setup sane defaults. The module hook is called
 * right after, so that, the user can change the defaults, i.e. when one needs more than 1
 * input/output structure.
 *
 *  module_taskpool_init()
 *  hello_taskpool_init()
 *
 * @in_group
 * Not required, hook performed right after prototype
 *
 * @in_group
 * All nodes
 * 
 */
int module_taskpool_init(Pool *p, Setup *s) {
  p->rank = 2;
  p->input_rank = 1;
  p->output_rank = 1;
}

/**
 * @function
 * Defines the storage layout
 *
 * Storage scheme:
 * /Pools
 *  - 0
 *    - board
 *    - data/master
 *    - data/tmp etc.
 * /Pools/0/data/master
 *
 * It is possible to change the layout for each Pool, according to the Pool data
 *
 * @after
 * The memory allocation and file layout is performed. To customize it, the user has the
 * Setup (already performed) and Init hooks
 *
 * @in_group
 * Not required, hook performed right after prototype
 *
 * @in_group
 * All nodes
 *
 * module_taskpool_storage()
 * hello_taskpool_storage()
 */
int module_taskpool_storage(Pool *p, Setup *s) {

  p.path = (char*) p->pid;

  p->input[0].path = "/data/tmp"; 
  p->input[0].rank = 2;
  p->input[0].dimsf[0] = 10;
  p->input[0].dimsf[1] = 9;
  p->input[0].dimsm[0] = 10;
  p->input[0].dimsm[1] = 1;
  p->input[0].hdf = FALSE; // whether to store data or not

  p->output[0].path = "/data/master";
  p->output[0].rank = 2;
  p->output[0].dimsf[0] = 16;
  p->output[0].dimsf[1] = p->dimsf[0] * p->dimsf[1];
  p->output[0].dimsm[0] = 16;
  p->output[0].dimsm[1] = 1;
  p->output[0].hdf = TRUE;
}

/**
 * @function
 * Calculates proper offsets and strides for the storage
 *
 * Example:
 * For a dynamical map we create output dataset of NumOfTask * output_length. Each result
 * should be stored with offset of TaskID (number of current task), for easy gnuplot
 * processing
 *
 * Example:
 * We create 2D Pool and 2D output. Each Task returns only 1 value which should be stored
 * one-by-one to Pool. In such case, we define offsets as:
 * offsets[0] = t.coord[0];
 * offsets[1] = t.coord[1];
 *
 * @in_group
 * Master only
 */
int module_task_storage_offsets(Pool *p, Setup *s, Task *t, hid_t offsets, hid_t strides) {
  offsets[0] = t.id;
  offsets[1] = 0;
}

/**
 * Example: Dynamical map
 *
 * We don't need Init and Setup hooks. We have to adjust only one storage variable. The
 * output will be compatible with the old Mechanic:
 *
 * /Pools/0/data/master
 *
 */
int hello_taskpool_storage(Pool *p, Setup *s) {
  p->output[0].dimsf = 10;
  p->output[0].dimsm = 10;
}

/**
 * @function
 * Prepares the taskpool for the run
 *
 * The Data *in and *out structures are broadcasted to all nodes right after this function
 * is performed. You can setup here some initial conditions, without MPI/HDF knowledge
 *
 * @in_group
 * Master only
 */
int module_taskpool_prepare(Pool *p, Setup *s, Data *in, Data *out) {

}

/**
 * @function
 * Decides whether to continue taskpool loop or finish the simulations
 *
 * @in_group
 * Master only
 */
int module_taskpool_postprocess(Pool *p, Setup *s, Data *in, Data *out) {

}
