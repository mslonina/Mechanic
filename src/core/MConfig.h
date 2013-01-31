/**
 * @file
 * Runtime configuration interface
 */
#ifndef MECHANIC_CONFIG_H
#define MECHANIC_CONFIG_H

#include "mechanic.h"

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
#define CONFIG_MSG_NONAMESPACE "No namespace has been specified"
#define CONFIG_MSG_UNKNOWN_NAMESPACE "Unknown namespace"

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

/**
 * Configuration helpers
 */

/* Search and modify */
config* ConfigFindOption(char* var, configNamespace* current);
config* ModifyOption(char* space, char* var, char* value, int type, configNamespace* head);

/* Converters */
int Option2Int(char* space, char* var, configNamespace* head);
long Option2Long(char* space, char* var, configNamespace* head);
float Option2Float(char* space, char* var, configNamespace* head);
double Option2Double(char* space, char* var, configNamespace* head);
char* Option2String(char* space, char* var, configNamespace* head);

#endif

