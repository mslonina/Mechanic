/**
 * @file
 * The Arnold Web module for Mechanic: The Mechanic part
 * 
 * Working code for the Mechanic-0.13 master branch
 */

/**
 * Mechanic-0.13 short user guide
 * ------------------------------
 *
 * ## Main features
 *
 * - The Pool-oriented task management, you may define as many task pools as needed, on
 *   the fly (see PoolProcess())
 * - Custom setup, both for core and the module, only one config file is used (see
 *   Setup())
 * - Custom storage, both for core and the module (see Storage())
 * - No MPI and HDF5 knowledge required (i.e. the PoolPrepare() does MPI_Broadcast under
 *   the hood)
 * - Non-blocking communication between nodes
 *
 *
 * ## The Pool Loop explained
 *
 * After the core and the module are bootstrapped, the Mechanic enters the four-step pool
 * loop:
 *
 *  1. PoolPrepare()
 *    All nodes enter the PoolPrepare(). Data of all pools is passed to this
 *    function, so that we may prepare specifically the current pool. The current pool
 *    data is broadcasted to all nodes and stored in the master datafile.
 *  2. The Task Loop
 *    All nodes enter the task loop. The master node prepares the task during
 *    TaskPrepare(). Each worker receives a task, and calls the TaskProcess(). The task
 *    data, marked with use_hdf = 1 are received by the master node after the
 *    TaskProcess() is finished and stored during the CheckpointProcess().
 *  3. The Checkpoint
 *    The CheckpointPrepare() function might be used to adjust the received data (2D array
 *    of packed data, each row is a separate task). The pool data might be adjusted here
 *    as well, the data is stored again during CheckpointProcess().
 *  4. PoolProcess()
 *    After the task loop is finished, the PoolProcess() is used to decide whether to
 *    continue the pool loop or not. The data of all pools is passed to this function. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Include Mechanic datatypes and function prototypes
 */
#include "MMechanic2.h"

/**
 * Include module-related stuff
 */
#include "mechanic_module_web.h"

/**
 * @function
 * Implements Init()
 *
 * This function is used to setup some sane defaults, you should use it only, if the core
 * defaults are not enough to handle your simulation. The core defaults are:
 *
 * - options = 128 -- max number of setup options
 * - pools = 64 -- max number of pools
 * - banks_per_pool = 8 -- memory banks per pool
 * - bansk_per_task = 8 -- memory banks per task
 */
int Init(init *i) {
  i->options = 24;
  i->banks_per_pool = 2;
  i->banks_per_task = 3;
  i->pools = 25;
  
  return TASK_SUCCESS;
}
/**
 * @function
 * Implements Setup()
 *
 * This function uses the LRC API for setup options. Options are merged with the core. The
 * syntax is:
 *
 * s->options[0] = (LRC_configDefaults) {namespace, variable, value, type};
 *
 * where
 *
 * - namespace - the name of the configuration namespace (string)
 * - variable - the name of the variable (string)
 * - value - the default value (string)
 * - type - the variable type (LRC_INT, LRC_FLOAT, LRC_DOUBLE, LRC_STRING)
 *
 * The options table must finish with LRC_OPTIONS_END.
 *
 * The configuration file might be used then, in a sample form (only one configuration
 * file both for core and module):
 *
 * [core]
 * name = arnoldweb
 * xres = 2048
 * yres = 2048
 * checkpoint = 4
 *
 * [arnold]
 * step = 0.3
 * tend = 2000.0
 * xmin = 0.95
 * xmax = 1.05
 * ymin = 0.95
 * ymax = 1.05
 * driver = 2
 * eps = 0.0
 * epsnax = 0.1
 * eps_interval = 0.02
 *
 * The final run configuration is stored in the master datafile.
 */
int Setup(setup *s) {

  s->options[0] = (LRC_configDefaults) {"arnold", "step", "0.25", LRC_DOUBLE};
  s->options[1] = (LRC_configDefaults) {"arnold", "tend", "1000.0", LRC_DOUBLE};
  s->options[2] = (LRC_configDefaults) {"arnold", "xmin", "0.8", LRC_DOUBLE};
  s->options[3] = (LRC_configDefaults) {"arnold", "xmax", "1.2", LRC_DOUBLE};
  s->options[4] = (LRC_configDefaults) {"arnold", "ymin", "0.8", LRC_DOUBLE};
  s->options[5] = (LRC_configDefaults) {"arnold", "ymax", "1.2", LRC_DOUBLE};
  s->options[6] = (LRC_configDefaults) {"arnold", "eps", "0.0", LRC_DOUBLE};
  s->options[7] = (LRC_configDefaults) {"arnold", "driver", "1", LRC_INT};
  s->options[8] = (LRC_configDefaults) {"arnold", "eps_interval", "0.01", LRC_DOUBLE};
  s->options[9] = (LRC_configDefaults) {"arnold", "epsmax", "0.04", LRC_DOUBLE};
  s->options[10] = (LRC_configDefaults) {LRC_OPTIONS_END};

  return TASK_SUCCESS;
}

/**
 * Implements Storage()
 *
 * This function is used to create the storage layout of your run. You define here all
 * datasets used during pool/task processing. The storage scheme is as follows:
 *
 * - storage[0].layout.path - the dataset name
 * - storage[0].layout.rank - the dataset rank (MAX_RANK = 2)
 * - storage[0].layout.dim - the dataset dimensions
 * - storage[0].layout.use_hdf - whether use hdf5 storage or not
 * - storage[0].layout.dataspace_type - the type of the hdf5 dataspace for the dataset
 *   (H5S_SIMPLE)
 * - storage[0].layout.datatype - the datatype of data in the dataset (H5T_NATIVE_DOUBLE)
 * - storage[0].layout.storage_type - the type of storage for the dataset
 *   - STORAGE_BASIC to store whole dataset at once,
 *   - STORAGE_PM3D to store data in a Gnuplot PM3D ready format
 *   - STORAGE_BOARD to store data in a task-pool dimension way, which is ready to process
 *   with Matplotlib
 *
 * The data may be access as follows:
 *
 * - storage[0].data[0][0]
 * - storage[0].data[0][1] etc.
 *
 * The storage scheme applies to the pool global data as well as task data:
 *
 * - p->storage[0]
 * - p->task->storage[0]
 * - p->task->storage[1]
 *
 * The task storage is independent on the pool storage.
 */
int Storage(pool *p, setup *s) {

  /** 
   * Path: /Pools/pool-ID/pool-data 
   *
   * The pool global data. Each dataset is stored in a pool-ID group, where ID is the
   * unique pool ID. The STORAGE_BASIC mode may be only used here.
   *
   */
  p->storage[0].layout.path = "pool-data";
  p->storage[0].layout.rank = 2;
  p->storage[0].layout.dim[0] = 1;
  p->storage[0].layout.dim[1] = 6;
  p->storage[0].layout.use_hdf = 1;
  p->storage[0].layout.dataspace_type = H5S_SIMPLE;
  p->storage[0].layout.datatype = H5T_NATIVE_DOUBLE;
  p->storage[0].layout.storage_type = STORAGE_BASIC;
  p->storage[0].layout.sync = 1;
  
  /**
   * Path: /Pools/pool-ID/Tasks/input 
   *
   * The task dataset. The data will be stored in the pool-ID/Tasks group as a
   * Gnuplot-PM3D ready dataset. This dataset is used for the initial
   * condition. Each worker node receives the 1x6 inital state array. Since the storage is
   * STORAGE_PM3D, the size of the output dataset is pool_size * dim[1].
   */
  p->task->storage[0].layout.path = "input";
  p->task->storage[0].layout.rank = 2;
  p->task->storage[0].layout.dim[0] = 1;
  p->task->storage[0].layout.dim[1] = 6;
  p->task->storage[0].layout.use_hdf = 1;
  p->task->storage[0].layout.dataspace_type = H5S_SIMPLE;
  p->task->storage[0].layout.datatype = H5T_NATIVE_DOUBLE;
  p->task->storage[0].layout.storage_type = STORAGE_PM3D;

  /**
   * Path: /Pools/pool-ID/Tasks/result
   *
   * The task result dataset, similar to the input one. The result consists of the
   * location (coordinates) of the task and the MEGNO.
   */
  p->task->storage[1].layout.path = "result";
  p->task->storage[1].layout.rank = 2;
  p->task->storage[1].layout.dim[0] = 1;
  p->task->storage[1].layout.dim[1] = 4;
  p->task->storage[1].layout.use_hdf = 1;
  p->task->storage[1].layout.dataspace_type = H5S_SIMPLE;
  p->task->storage[1].layout.datatype = H5T_NATIVE_DOUBLE;
  p->task->storage[1].layout.storage_type = STORAGE_PM3D;

  return TASK_SUCCESS;
}

/**
 * @function
 * Implements TaskPrepare()
 *
 * The default task mapping follows the HDF5 storage scheme.
 * The mapping starts from the top left corner:
 *
 * (0,0) (0,1) (0,2) (0,3) ...
 * (1,0) (1,1) (1,2) (1,3)
 * (2,0) (2,1) (2,2) (2,3)
 *  ...
 *
 * The current task location is available at t->location array. The task pool resolution
 * is available at p->board->layout.dim array. The pool_size is a multiplication of
 * p->board->layout.dim[i], i < p->board->layout.rank. 
 *
 */
int TaskPrepare(pool *p, task *t, setup *s) {
  double xmin, xmax, ymin, ymax;

  /* Global map range (equivalent of frequencies space) */
  xmin = LRC_option2double("arnold", "xmin", s->head);
  xmax = LRC_option2double("arnold", "xmax", s->head);
  ymin = LRC_option2double("arnold", "ymin", s->head);
  ymax = LRC_option2double("arnold", "ymax", s->head);

  /* Initial condition - angles */
  t->storage[0].data[0][0] = 0.131;
  t->storage[0].data[0][1] = 0.132;
  t->storage[0].data[0][2] = 0.212;

  /* Map coordinates */  
  t->storage[0].data[0][3] = xmin + t->location[1]*(xmax-xmin)/(1.0*p->board->layout.dim[1]);
  t->storage[0].data[0][4] = ymin + t->location[0]*(ymax-ymin)/(1.0*p->board->layout.dim[0]);
  t->storage[0].data[0][5] = 0.01;  

  return TASK_SUCCESS;
}

/**
 * @function
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double err, xv[6], tend, step, eps, result;
  int driver = 0;

  step = LRC_option2double("arnold", "step", s->head);
  step  = step*(pow(5,0.5)-1)/2.0;
  tend = LRC_option2double("arnold", "tend", s->head);
  eps = p->storage[0].data[0][0]; 

  driver = LRC_option2int("arnold", "driver", s->head);

  /* Initial data */  
  xv[0] = t->storage[0].data[0][0];
  xv[1] = t->storage[0].data[0][1];
  xv[2] = t->storage[0].data[0][2];
  xv[3] = t->storage[0].data[0][3];
  xv[4] = t->storage[0].data[0][4];
  xv[5] = t->storage[0].data[0][5];  

  /* Numerical integration goes here */
  if (driver == 1) result = smegno2(xv, step, tend, eps, &err);
  if (driver == 2) result = smegno3(xv, step, tend, eps, &err);

  /* Assign the master result */
  t->storage[1].data[0][0] = xv[3];
  t->storage[1].data[0][1] = xv[4];
  t->storage[1].data[0][2] = result;
  t->storage[1].data[0][3] = err;

  return TASK_SUCCESS;
}

/**
 * Implements PoolPrepare()
 *
 * You may use the **all array to access data of all previous pools. The data of the
 * current pool p are broadcasted and stored (if use_hdf = 1) right after PoolPrepare().
 */
int PoolPrepare(pool **all, pool *p, setup *s) {
  double eps, epsmin, epsint;
  epsmin  = LRC_option2double("arnold", "eps", s->head);
  epsint  = LRC_option2double("arnold", "eps_interval", s->head);
  eps = epsmin + (double)p->pid * epsint; 

  p->storage[0].data[0][0] = eps; 
  return TASK_SUCCESS;
}
 
/**
 * Implements PoolProcess()
 *
 * You may the **all array to access data of all previous pools. This function should
 * return POOL_CREATE_NEW when the new pool will be created or POOL_FINALIZE otherwise.
 */
int PoolProcess(pool **all, pool *p, setup *s) {
  double eps, epsmax;
  eps = p->storage[0].data[0][0];
  epsmax  = LRC_option2double("arnold", "epsmax", s->head);

  if (eps < epsmax) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}
