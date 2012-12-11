#ifndef MECHANIC_CONFIG_H
#define MECHANIC_CONFIG_H

#include "mechanic.h"

#define CONFIG_GROUP "config"
#define CONFIG_HDF5_DATATYPE "Config"

/**
 * @def CONFIG_ERR_CONFIG_SYNTAX
 * @brief Main message for config syntax error. 
 * 
 * @def CONFIG_ERR_MISSING_VAR
 * @brief Message for missing variable error.
 * 
 * @def CONFIG_ERR_MISSING_VAL
 * @brief Message for missing value error.
 * 
 * @def CONFIG_ERR_MISSING_SEP
 * @brief Message for missing separator error.
 * 
 * @def CONFIG_ERR_MISSING_BRACKET
 * @brief Message for namespace error.
 * 
 * @def CONFIG_ERR_TOOMANY_SEP
 * @brief Message for toomany separators error.
 * 
 * @def CONFIG_ERR_WRONG_INPUT
 * @brief Message for wrong user input.
 *
 * @def CONFIG_ERR_UNKNOWN_VAR
 * @brief Message for unknown variable error.
 */

enum configMessagesType {
  CONFIG_ERR_CONFIG_SYNTAX,
  CONFIG_ERR_WRONG_INPUT,
  CONFIG_ERR_UNKNOWN_VAR,
  CONFIG_ERR_FILE_OPEN,
  CONFIG_ERR_FILE_CLOSE,
  CONFIG_ERR_HDF
} configMessages;

#define CONFIG_MSG_CONFIG_SYNTAX "Config file syntax error"
#define CONFIG_MSG_MISSING_VAR "Missing variable name"
#define CONFIG_MSG_MISSING_VAL "Missing value"
#define CONFIG_MSG_MISSING_SEP "Missing separator"
#define CONFIG_MSG_MISSING_BRACKET "Missing bracket in namespace"
#define CONFIG_MSG_TOOMANY_SEP "Too many separators"
#define CONFIG_MSG_WRONG_INPUT "Wrong input value type"
#define CONFIG_MSG_UNKNOWN_VAR "Unknown variable"
#define CONFIG_MSG_FILE_OPEN "File open error"
#define CONFIG_MSG_HDF "HDF5 error"
#define CONFIG_MSG_NONAMESPACE "No namespace has been specified"
#define CONFIG_MSG_UNKNOWN_NAMESPACE "Unknown namespace"

/**
 * @var typedef struct ccd_t
 * @brief Helper datatype used for HDF5 storage
 *
 * @param name
 *  Name of the variable
 * 
 * @param value
 *  Value of the variable
 *
 * @param type
 *  Type of the variable
 */
typedef struct{
  char name[CONFIG_LEN];
  char value[CONFIG_LEN];
  int type;
} ccd_t;

configNamespace* ConfigAssignDefaults(options* cd);
void ConfigCleanup(configNamespace* head);
void ConfigPrintAll(configNamespace* head);

int ConfigAllOptions(configNamespace* head);
int ConfigCountOptions(char* space, configNamespace* head);
configNamespace* ConfigFindNamespace(char* space, configNamespace* head);
int ConfigCountDefaultOptions(options *in);
int ConfigMergeDefaults(options *in, options *add);
options* ConfigHead2Struct(configNamespace *head);
int ConfigHead2StructNoalloc(configNamespace *head, options *c);
int ConfigItoa(char* deststr, int value, int type);
char* ConfigTrim(char*);
void ConfigMessage(int line, int type, char* message);
char* ConfigNameTrim(char*);
int ConfigCharCount(char*, char*);
int ConfigMatchType(char*, char*, options*, int);
int ConfigCheckType(char*, int);
int ConfigIsAllowed(int);
int ConfigCheckName(char*, options*, int);
configNamespace* ConfigNewNamespace(char* cfg);
configNamespace* ConfigLastLeaf(configNamespace* head);

int ConfigAsciiParser(FILE* file, char* sep, char* comm, configNamespace* head);
int ConfigAsciiWriter(FILE* file, char* sep, char* comm, configNamespace* head);
int ConfigHDF5Parser(hid_t file_id, char* group_name, configNamespace* head);
int ConfigHDF5Writer(hid_t file_id, char* group_name, configNamespace* head);

#endif
