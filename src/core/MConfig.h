#ifndef MECHANIC_CONFIG_H
#define MECHANIC_CONFIG_H

#include "Mechanic2.h"

#define CONFIG_GROUP "config"
#define CONFIG_HDF5_DATATYPE "Config"

/**
 * @def LRC_E_CONFIG_SYNTAX
 * @brief Main message for config syntax error. 
 * 
 * @def LRC_E_MISSING_VAR
 * @brief Message for missing variable error.
 * 
 * @def LRC_E_MISSING_VAL
 * @brief Message for missing value error.
 * 
 * @def LRC_E_MISSING_SEP
 * @brief Message for missing separator error.
 * 
 * @def LRC_E_MISSING_BRACKET
 * @brief Message for namespace error.
 * 
 * @def LRC_E_TOOMANY_SEP
 * @brief Message for toomany separators error.
 * 
 * @def LRC_E_WRONG_INPUT
 * @brief Message for wrong user input.
 *
 * @def LRC_E_UNKNOWN_VAR
 * @brief Message for unknown variable error.
 */

enum LRC_messages_type{
  LRC_ERR_CONFIG_SYNTAX,
  LRC_ERR_WRONG_INPUT,
  LRC_ERR_UNKNOWN_VAR,
  LRC_ERR_FILE_OPEN,
  LRC_ERR_FILE_CLOSE,
  LRC_ERR_HDF
} LRC_messages;

#define LRC_MSG_CONFIG_SYNTAX "Config file syntax error"
#define LRC_MSG_MISSING_VAR "Missing variable name"
#define LRC_MSG_MISSING_VAL "Missing value"
#define LRC_MSG_MISSING_SEP "Missing separator"
#define LRC_MSG_MISSING_BRACKET "Missing bracket in namespace"
#define LRC_MSG_TOOMANY_SEP "Too many separators"
#define LRC_MSG_WRONG_INPUT "Wrong input value type"
#define LRC_MSG_UNKNOWN_VAR "Unknown variable"
#define LRC_MSG_FILE_OPEN "File open error"
#define LRC_MSG_HDF "HDF5 error"
#define LRC_MSG_NONAMESPACE "No namespace has been specified"
#define LRC_MSG_UNKNOWN_NAMESPACE "Unknown namespace"


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
