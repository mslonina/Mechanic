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
 *     mpirun -np 2 mechanic -p ex_poolsetup --help
 *
 * ### Getting the run-time configuration
 *
 * Configuration options are stored as char strings in a dynamic linked-list,
 * and you may access them by using the `setup->head` pointer and following functions:
 *
 * - `Option2Int(namespace, variable long name, setup->head)` for `C_INT`, `C_VAL`
 * - `Option2Float(namespace, variable long name, setup->head)` for `C_DOUBLE`
 * - `Option2Double(namespace, variable long name, setup->head)` for `C_FLOAT`
 * - `Option2String(namespace, variable long name, setup->head)` for `C_STRING`
 *
 * You can modify an option during run-time by calling:
 *
 *     ModifyOption(namespace, variable, new value, new type, setup->head);
 *
 * ### Accessing the core options
 *
 * Core options are handled in the same way the module options are. You can access them by
 * using "core" namespace.
 *
 * ### Configuration file
 *
 * The Mechanic uses only the one configuration file (both for the core and the module).
 * The file has the following scheme:
 *
 *     [namespace]
 *     variable = value # an inline comment
 *     # full line comment
 *
 * For example:
 *
 *     [mymodule]
 *     dlimit = 99.8
 *     ilimit = 15
 *     host = myhost
 *
 * To use configuration file try:
 *
 *     mpirun -np 2 mechanic -p ex_setup --config=myconfigfile
 *
 * ### Future
 *
 * The configuration options are marked to become full HDF5 attributes, so the entire API
 * of accessing options is a subject to change. Starting from 2.2.6 release, all defined
 * options (both core and module) are available as attributes to the task board dataset,
 * i.e. /Pools/last/board.
 */

#include "mechanic.h"

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
 * Implements Prepare()
 */
int Prepare(int node, char *masterfile, setup *s) {
  int max_pools, ilimit, dtrue;
  double dlimit;

  max_pools = Option2Int("mymodule", "max-pools", s->head);
  ilimit = Option2Int("mymodule", "ilimit", s->head);
  dtrue = Option2Int("mymodule", "dtrue", s->head);
  dlimit = Option2Double("mymodule", "dlimit", s->head);

  if (node == MASTER) {
    Message(MESSAGE_COMMENT, "Options are: \n");
    Message(MESSAGE_COMMENT, "--max-pools = %d\n", max_pools);
    Message(MESSAGE_COMMENT, "--ilimit = %d\n", ilimit);
    Message(MESSAGE_COMMENT, "--dtrue = %d\n", dtrue);
    Message(MESSAGE_COMMENT, "--dlimit = %f\n", dlimit);
    Message(MESSAGE_COMMENT, "--host = %s\n", Option2String("mymodule", "host", s->head));
    Message(MESSAGE_COMMENT, "\n");
  }

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **all, pool *current, setup *s) {
  
  // Modify an option depending on the task pool
  if (current->pid == 2) {
    ModifyOption("mymodule", "ilimit", "34", C_INT, s->head);
  }
  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **all, pool *current, setup *s) {
  int max_pools, ilimit, dtrue;
  double dlimit;

  // Getting the integer data
  max_pools = Option2Int("mymodule", "max-pools", s->head);
  ilimit = Option2Int("mymodule", "ilimit", s->head);
  dtrue = Option2Int("mymodule", "dtrue", s->head);
  dlimit = Option2Double("mymodule", "dlimit", s->head);

  Message(MESSAGE_COMMENT, "Pool %d processed with option ilimit %d\n", current->pid, ilimit);

  if (current->pid < max_pools) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}
