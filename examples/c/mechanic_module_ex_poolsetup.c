/**
 * Changing the runtime configuration per pool
 * ===========================================
 *
 * In this example we show how to change the runtime configuration per task pool
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_poolsetup.c -o libmechanic_module_ex_poolsetup.so
 *
 *
 * Using the module
 * ----------------
 *
 *     mpirun -np 2 mechanic -p ex_poolsetup 
 *
 */
#include "mechanic.h"

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

