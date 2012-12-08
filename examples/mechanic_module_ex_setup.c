/**
 * Using the Init()/Setup() hooks
 * ==============================
 *
 * The Setup() hook is to define the configuration options. The
 * options are automatically merged with the core options, added to the popt table
 * (command line) and stored in the master datafile
 *
 * The Init() hook is used to define some low-level Mechanic variables, such as number of
 * storage banks or configuration options.
 *
 * Using the module
 * ----------------
 *
 * To get the available configuration options try:
 *
 *     mpirun -np 2 mechanic2 -p ex_setup --help
 *
 * Config API short introduction
 * -----------------------------
 *
 * ### Defining options
 *
 * Configuration options are defined in a following structure:
 *
 *     (options) {
 *      .space,
 *      .name,
 *      .value,
 *      .short_name,
 *      .type,
 *      .description
 *     }
 *
 * where:
 * - `space` - configration namespace (string)
 * - `name` - long name of the variable (string)
 * - `short_name` - the short name of the variable (int, may be `'\0'`)
 * - `value` - the default value (string)
 * - `type` - the datatype of the variable: `C_INT`, `C_DOUBLE`, `C_FLOAT`, `C_VAL`, `C_STRING`
 * - `description` - description of a variable
 *
 * All options are automatically added to the Popt table and are available in the command
 * line (long and short names are used respectively).
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
 * ### Access the core options
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
 *     mpirun -np 2 mechanic2 -p ex_setup --config=myconfigfile
 *
 * ### Future
 *
 * The configuration options are marked to become full HDF5 attributes, so the entire API
 * of accessing options is a subject to change.
 */

#include "Mechanic2.h"

/**
 * Implements Init()
 *
 * The Init() hook allow to change the core defaults, such as:
 * - the maximum number of task pools
 * - the number of memory banks for a task
 * - the number of memory banks for a pool
 * - the number of configuration options
 * - the minimum number of CPU required to run the job
 */
int Init(init *i) {
  i->options = 24;
  i->pools = 128;
  i->banks_per_pool = 4;
  i->banks_per_task = 4;
  i->min_cpu_required = 2;

  return SUCCESS;
}

/**
 * Implements Setup()
 */
int Setup(setup *s) {
  s->options[0] = (options) {
    .space="mymodule", // The namespace of the configuration
    .name="max-pools", // The name of the variable
    .shortName='\0', // The short name of the variable (command line)
    .value="10", // Default value
    .type=C_INT, // Type of the variable
    .description="Maximum number of pools to evaluate (max 128)" // Description
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
    .name="debug",
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
  int max_pools, ilimit, debug;
  double dlimit;

  max_pools = Option2Int("mymodule", "max-pools", s->head);
  ilimit = Option2Int("mymodule", "ilimit", s->head);
  debug = Option2Int("mymodule", "debug", s->head);
  dlimit = Option2Double("mymodule", "dlimit", s->head);

  if (node == MASTER) {
    Message(MESSAGE_COMMENT, "Options are: \n");
    Message(MESSAGE_COMMENT, "--max-pools = %d\n", max_pools);
    Message(MESSAGE_COMMENT, "--ilimit = %d\n", ilimit);
    Message(MESSAGE_COMMENT, "--debug = %d\n", debug);
    Message(MESSAGE_COMMENT, "--dlimit = %f\n", dlimit);
    Message(MESSAGE_COMMENT, "--host = %s\n", Option2String("mymodule", "host", s->head));
    Message(MESSAGE_COMMENT, "\n");
  }

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 *
 * Here we decide whether to continue the task pool loop. If the max pools defined in the
 * Init() hook is reached, the task pool loop will be finished.
 */
int PoolProcess(pool **all, pool *current, setup *s) {
  int max_pools, ilimit, debug;
  double dlimit;

  // Getting the integer data
  max_pools = Option2Int("mymodule", "max-pools", s->head);
  ilimit = Option2Int("mymodule", "ilimit", s->head);
  debug = Option2Int("mymodule", "debug", s->head);
  dlimit = Option2Double("mymodule", "dlimit", s->head);

  Message(MESSAGE_COMMENT, "Pool %d processed\n", current->pid);

  if (current->pid < max_pools) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}
