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
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_setup.c -o libmechanic_module_ex_setup.so
 *
 *
 * Using the module
 * ----------------
 *
 * To get the available configuration options try:
 *
 *     mpirun -np 2 mechanic -p ex_setup --help
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
 * Configuration options (both core and the module) are stored as attributes to the task board dataset
 * and you may access them by using the following macros:
 *
 * - `MReadOption(pool, option, value)`, i.e.
 *    
 *    int ilimit;
 *    MReadOption(p, "ilimit", &ilimit);
 *
 * You can modify an option during run-time by calling:
 *
 * - `MWriteOption(pool, option, value)`, i.e.
 *
 *    int ilimit = 45;
 *    MWriteOption(p, "ilimit", &ilimit);
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
 */

#include "mechanic.h"

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

int Storage(pool *current, setup *s) {
  int max_pools, ilimit, dtrue;
  double dlimit;
  char host[CONFIG_LEN];
  
  MReadOption(current, "ilimit", &ilimit);
  MReadOption(current, "max-pools", &max_pools);
  MReadOption(current, "dtrue", &dtrue);
  MReadOption(current, "dlimit", &dlimit);
  MReadOption(current, "host", &host);

  if (current->pid == 0) {
    Message(MESSAGE_COMMENT, "Options are: \n");
    Message(MESSAGE_COMMENT, "--max-pools = %d\n", max_pools);
    Message(MESSAGE_COMMENT, "--ilimit = %d\n", ilimit);
    Message(MESSAGE_COMMENT, "--dtrue = %d\n", dtrue);
    Message(MESSAGE_COMMENT, "--dlimit = %f\n", dlimit);
    Message(MESSAGE_COMMENT, "--host = %s\n", host);
    Message(MESSAGE_COMMENT, "\n");
  }

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 *
 * Here we decide whether to continue the task pool loop. If the max pools defined in the
 * Init() hook is reached, the task pool loop will be finished.
 *
 * We read options, and print the primary setup on the first pool. Options may be modified
 * per pool using `MWriteOption` macro.
 */
int PoolProcess(pool **all, pool *current, setup *s) {
  int max_pools, ilimit, dtrue;
  double dlimit;
  char host[CONFIG_LEN];

  MReadOption(current, "ilimit", &ilimit);
  MReadOption(current, "max-pools", &max_pools);
  MReadOption(current, "dtrue", &dtrue);
  MReadOption(current, "dlimit", &dlimit);
  MReadOption(current, "host", &host);

  if (current->pid == 0) {
    Message(MESSAGE_COMMENT, "Options are: \n");
    Message(MESSAGE_COMMENT, "--max-pools = %d\n", max_pools);
    Message(MESSAGE_COMMENT, "--ilimit = %d\n", ilimit);
    Message(MESSAGE_COMMENT, "--dtrue = %d\n", dtrue);
    Message(MESSAGE_COMMENT, "--dlimit = %f\n", dlimit);
    Message(MESSAGE_COMMENT, "--host = %s\n", host);
    Message(MESSAGE_COMMENT, "\n");
  }

  Message(MESSAGE_COMMENT, "Pool %d processed\n", current->pid);

  if (current->pid < max_pools) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}
