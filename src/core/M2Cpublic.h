/**
 * @file
 * Runtime configuration (public API)
 */
#ifndef MECHANIC_M2C_PUBLIC_H
#define MECHANIC_M2C_PUBLIC_H

#include <popt.h>

/**
 * The configuration
 */
#define OPTIONS_END {.space="", .name="", .shortName='\0', .value="", .description="", .type=0} /**< Options default initializer */

#define C_VAL POPT_ARG_VAL /**< int */
#define C_INT POPT_ARG_INT /**< int */
#define C_FLOAT POPT_ARG_FLOAT /**< float */
#define C_DOUBLE POPT_ARG_DOUBLE /**< double */
#define C_STRING POPT_ARG_STRING /**< string */
#define C_LONG POPT_ARG_LONG /**< long */

#define CONFIG_MAX_LINE_LENGTH 512 /**< Maximum line length in the config file */
#define CONFIG_LEN 128 /**< Maximum config filename length */
#define CONFIG_NULL '\0' /**< Null */

/**
 * @struct config
 * Internal options struct
 */
typedef struct config{
  char name[CONFIG_LEN]; /**< The name of the variable */
  char value[CONFIG_LEN]; /**< The value of the variable */
  int type; /**< The datatype of the variable */
  struct config *next; /**< The next option pointer */
} config;

/**
 * @struct configNamespace
 * The internal namespace struct
 */
typedef struct configNamespace{
  char space[CONFIG_LEN]; /**< The name of the namespace */
  config *options; /**< The options structure for given namespace */
  struct configNamespace *next; /**< The pointer to the next namespace */
} configNamespace;

/**
 * @struct options
 * The configuration options
 */
typedef struct {
  char space[CONFIG_LEN]; /**< The name of the configuration namespace */
  char name[CONFIG_LEN]; /**< The name of the variable */
  char shortName; /**< The short name of the variable */
  char value[CONFIG_LEN]; /**< The default value of the variable */
  char description[4*CONFIG_LEN]; /**< The description of the variable */
  int type; /**< The datatype of the variable */
} options;

/**
 * @struct popt
 * Popt
 */
typedef struct {
  struct poptOption *popt; /**< The Popt options */
  poptContext poptcontext; /**< The Popt context */
  char **string_args; /**< String arguments received from Popt */
  int *val_args; /**< Value arguments received from Popt */
  int *int_args; /**< Integer arguments received from Popt */
  long *long_args; /**< Long integer arguments received from Popt */
  float *float_args; /**< Float arguments received from Popt */
  double *double_args; /**< Double arguments received from Popt */
} popt;

/**
 * @struct setup
 * The setup structure, combines Config and Popt
 */
typedef struct {
  options *options; /**< The Config default options table */
  configNamespace *head; /**< The Config options linked list */
  popt *popt; /**< The popt options, @see popt */
} setup;

/**
 * @macro
 * Read the attribute for the pool task board (runtime configuration)
 */
#define MReadOption(_mobject, _mattr_name, _mdata)\
  if (_mobject) {\
    int _maindex, _mmstat;\
    _maindex = GetAttributeIndex(_mobject->board->attr, _mattr_name);\
    if (_maindex < 0) {\
      Message(MESSAGE_ERR, "MReadOption: Option '%s' could not be found\n",\
        _mattr_name);\
      Error(CORE_ERR_MEM);\
    }\
    _mmstat = ReadAttr(&_mobject->board->attr[_maindex], _mdata);\
    CheckStatus(_mmstat);\
  } else {\
    Message(MESSAGE_ERR, "MReadOption: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

/**
 * @macro
 * Write the attribute for the pool task board (runtime configuration)
 */
#define MWriteOption(_mobject, _mattr_name, _mdata)\
  if (_mobject) {\
    int _maindex, _mmstat;\
    _maindex = GetAttributeIndex(_mobject->board->attr, _mattr_name);\
    if (_maindex < 0) {\
      Message(MESSAGE_ERR, "MWriteOption: Option '%s' could not be found\n",\
        _mattr_name);\
      Error(CORE_ERR_MEM);\
    }\
    _mmstat = WriteAttr(&_mobject->board->attr[_maindex], _mdata);\
    CheckStatus(_mmstat);\
  } else {\
    Message(MESSAGE_ERR, "MWriteOption: Invalid object\n");\
    Error(CORE_ERR_MEM);\
  }

#endif
