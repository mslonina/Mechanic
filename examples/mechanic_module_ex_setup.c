/**
 * Using the Init()/Setup() hooks
 * ==============================
 *
 * The Setup() hook uses Libreadconfig public API to define the configuration options. The
 * options are automatically merged with the core options, added to the popt table
 * (command line) and stored in the master datafile
 *
 * Using the module
 * ----------------
 *
 * To get the available configuration options try:
 *
 *     mpirun -np 2 mechanic2 -p setup --help
 *
 * LRC short introduction
 * ----------------------
 *
 * Libreadconfig (LRC) is a library developed to handle configuration files.
 *
 * ### Defining options
 *
 * LRC-type options are defined in a following structure:
 *
 *     (LRC_configDefaults) {
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
 * - `type` - the datatype of the variable: `LRC_INT`, `LRC_DOUBLE`, `LRC_FLOAT`, `LRC_VAL`, `LRC_STRING`
 * - `description` - description of a variable
 *
 * All options are automatically added to the Popt table and are available in the command
 * line (long and short names are used respectively).
 *
 * ### Getting the run-time configuration
 *
 * LRC-type options are stored as char strings in a dynamic linked-list,
 * and you may access them by using the `setup->head` pointer and following functions:
 *
 * - `LRC_option2int(namespace, variable long name, setup->head)` for `LRC_INT`, `LRC_VAL`
 * - `LRC_option2float(namespace, variable long name, setup->head)` for `LRC_DOUBLE`
 * - `LRC_option2double(namespace, variable long name, setup->head)` for `LRC_FLOAT`
 *
 * You can modify an option during run-time by calling:
 *
 *     LRC_modifyOption(namespace, variable, new value, new type, setup->head);
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
 *     mpirun -np 2 mechanic2 -p setup --config=myconfigfile
 */

#include "MMechanic2.h"

/**
 * Implements Init()
 *
 * The Init() hook allow to change the core defaults, such as:
 * - the maximum number of task pools
 * - the number of memory banks for a task
 * - the number of memory banks for a pool
 * - the number of configuration options
 */
int Init(init *i) {
  i->options = 24;
  i->pools = 128;
  i->banks_per_pool = 4;
  i->banks_per_task = 4;

  return SUCCESS;
}

/**
 * Implements Setup()
 */
int Setup(setup *s) {
  s->options[0] = (LRC_configDefaults) {
    .space="mymodule", // The namespace of the configuration
    .name="max-pools", // The name of the variable
    .shortName='\0', // The short name of the variable (command line)
    .value="10", // Default value
    .type=LRC_INT, // Type of the variable
    .description="Maximum number of pools to evaluate (max 128)" // Description
  };
  s->options[1] = (LRC_configDefaults) {
    .space="mymodule",
    .name="dlimit",
    .shortName='l',
    .value="99.5",
    .type=LRC_DOUBLE,
    .description="Example double-type value",
  };
  s->options[2] = (LRC_configDefaults) {
    .space="mymodule",
    .name="ilimit",
    .shortName='i',
    .value="12",
    .type=LRC_INT,
    .description="Example integer-type value"
  };
  s->options[3] = (LRC_configDefaults) {
    .space="mymodule",
    .name="debug",
    .shortName='\0',
    .value="0",
    .type=LRC_VAL,
    .description="Example of boolean-type value"
  };
  s->options[4] = (LRC_configDefaults) {
    .space="mymodule",
    .name="host",
    .shortName='\0',
    .value="localhost",
    .type=LRC_STRING,
    .description="Example of string-type value"
  };
  // Options must end with this:
  s->options[5] = (LRC_configDefaults) LRC_OPTIONS_END;
  return SUCCESS;
}

/**
 * Implements Prepare()
 */
int Prepare(int node, char *masterfile, setup *s) {
  int max_pools, ilimit, debug;
  double dlimit;

  max_pools = LRC_option2int("mymodule", "max-pools", s->head);
  ilimit = LRC_option2int("mymodule", "ilimit", s->head);
  debug = LRC_option2int("mymodule", "debug", s->head);
  dlimit = LRC_option2double("mymodule", "dlimit", s->head);

  if (node == MASTER) {
    Message(MESSAGE_COMMENT, "Options are: \n");
    Message(MESSAGE_COMMENT, "--max-pools = %d\n", max_pools);
    Message(MESSAGE_COMMENT, "--ilimit = %d\n", ilimit);
    Message(MESSAGE_COMMENT, "--debug = %d\n", debug);
    Message(MESSAGE_COMMENT, "--dlimit = %f\n", dlimit);
    Message(MESSAGE_COMMENT, "--host = %s\n", LRC_getOptionValue("mymodule", "host", s->head));
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
  max_pools = LRC_option2int("mymodule", "max-pools", s->head);
  ilimit = LRC_option2int("mymodule", "ilimit", s->head);
  debug = LRC_option2int("mymodule", "debug", s->head);
  dlimit = LRC_option2double("mymodule", "dlimit", s->head);

  Message(MESSAGE_COMMENT, "Pool %d processed\n", current->pid);

  if (current->pid < max_pools) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}
