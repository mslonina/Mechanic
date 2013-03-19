/**
 * Changing the runtime configuration per pool and reading it on worker nodes
 * ==========================================================================
 *
 * In this example we show how to change the runtime configuration per task pool. We also
 * read the configuration on worker nodes with `LoopPrepare()` hook.
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_poolsetup2.c -o libmechanic_module_ex_poolsetup2.so
 *
 *
 * Using the module
 * ----------------
 *
 *     mpirun -np 2 mechanic -p ex_poolsetup2
 *
 */
#include "mechanic.h"

// sample structure
typedef struct myopts {
  int max_pools;
  double dlimit;
  int ilimit;
  int dtrue;
} myopts;

myopts runtime_opts;

/**
 * Implements Init()
 */
int Init(init *i) {
  i->pools = 12;
  return SUCCESS;
}

/**
 * Implements Setup()
 */
int Setup(setup *s) {
  s->options[0] = (options) {
    .space="mymodule",
    .name="max-pools",
    .shortName='\0',
    .value="10",
    .type=C_INT,
    .description="Maximum number of pools to evaluate (max 128)"
  };
  s->options[1] = (options) {
    .space="mymodule",
    .name="dlimit",
    .shortName='l',
    .value="99.5",
    .type=C_DOUBLE,
    .description="Example double-type value",
  };
  s->options[2] = (options) {
    .space="mymodule",
    .name="ilimit",
    .shortName='i',
    .value="12",
    .type=C_INT,
    .description="Example integer-type value"
  };
  s->options[3] = (options) {
    .space="mymodule",
    .name="dtrue",
    .shortName='\0',
    .value="0",
    .type=C_VAL,
    .description="Example of boolean-type value"
  };
  s->options[4] = (options) {
    .space="mymodule",
    .name="host",
    .shortName='\0',
    .value="localhost",
    .type=C_STRING,
    .description="Example of string-type value"
  };
  // Options must end with this:
  s->options[5] = (options) OPTIONS_END;
  return SUCCESS;
}

/**
 * Implements Storage()
 *
 * For testing purpose we will store some worker data
 */
int Storage(pool *p, void *s) {
  p->task->storage[0].layout = (schema) {
    .name = "result",
    .rank = TASK_BOARD_RANK,
    .dims[0] = 1,
    .dims[1] = 1,
    .dims[2] = 3,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
    .datatype = H5T_NATIVE_INT
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 *
 * The PoolPrepare hook is the best place to modify runtime configuration. After the hook
 * is called, all configuration options are broadcasted to all workers and stored as
 * attribtues to the task board dataset.
 */
int PoolPrepare(pool **all, pool *current, void *s) {
  int ilimit; 
  // Modify an option depending on the task pool
  if (current->pid == 2) {
    ilimit = 34;
    MWriteOption(current, "ilimit", &ilimit);
  }
  return SUCCESS;
}

/**
 * Implements LoopPrepare()
 *
 * Each node receives the full configuration (as HDF5 attributes) after the
 * `PoolPrepare()` is performed. The `LoopPrepare()` hook, which runs after the
 * `PoolPrepare()` may be used to convert the attribute-based configuration to local
 * runtime configuration.
 */
int LoopPrepare(int mpi_size, int node, pool **all, pool *p, void *s) {

  MReadOption(p, "max-pools", &runtime_opts.max_pools);
  MReadOption(p, "dlimit", &runtime_opts.dlimit);
  MReadOption(p, "ilimit", &runtime_opts.ilimit);
  MReadOption(p, "dtrue", &runtime_opts.dtrue);

  Message(MESSAGE_INFO, "Node %d options: %d %f %d %d\n", node,
      runtime_opts.max_pools, runtime_opts.dlimit, runtime_opts.ilimit, runtime_opts.dtrue); 
  
  return SUCCESS;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, void *s) {
  int result[1][1][3];

  result[0][0][0] = runtime_opts.ilimit + t->tid;
  result[0][0][1] = t->tid;
  result[0][0][2] = runtime_opts.max_pools - t->tid;

  MWriteData(t, "result", &result[0][0][0]);

  return TASK_FINALIZE;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **all, pool *current, void *s) {
  int max_pools, ilimit, dtrue;
  double dlimit;

  MReadOption(current, "ilimit", &ilimit);
  MReadOption(current, "max-pools", &max_pools);
  MReadOption(current, "dtrue", &dtrue);
  MReadOption(current, "dlimit", &dlimit);

  Message(MESSAGE_COMMENT, "Pool %d processed with option ilimit %d\n", current->pid, ilimit);

  if (current->pid < max_pools) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}

