/**
 * @file
 * Runtime configuration interface
 */
#ifndef MECHANIC_M2C_PRIVATE_H
#define MECHANIC_M2C_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "M2Cpublic.h"
#include "M2Epublic.h"

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

enum configMessagesType {
  CONFIG_ERR_CONFIG_SYNTAX,
  CONFIG_ERR_WRONG_INPUT,
  CONFIG_ERR_UNKNOWN_VAR,
  CONFIG_ERR_FILE_OPEN,
  CONFIG_ERR_FILE_CLOSE,
} configMessages;

int ConfigAllOptions(configNamespace *head);
int ConfigCountOptions(char *space, configNamespace *head);
int ConfigCountDefaultOptions(options *in);
int ConfigMergeDefaults(options *in, options *add);
int ConfigHead2StructNoalloc(configNamespace *head, options *c);

char* ConfigTrim(char *s);
char* ConfigNameTrim(char *name);
int ConfigCharCount(char *str, char *s);

configNamespace* ConfigAssignDefaults(options *cd);
configNamespace* ConfigNewNamespace(char *cfg);
configNamespace* ConfigFindNamespace(char *space, configNamespace *head);
configNamespace* ConfigLastLeaf(configNamespace *head);

options* ConfigHead2Struct(configNamespace *head);

int ConfigAsciiParser(FILE *file, char *sep, char *comm, configNamespace *head);
int ConfigAsciiWriter(FILE *file, char *sep, char *comm, configNamespace *head);

void ConfigMessage(int line, int type, char *message);
void ConfigPrintAll(configNamespace *head);
void ConfigCleanup(configNamespace *head);

/* Search and modify */
config* ConfigFindOption(char *var, configNamespace *current);
config* ModifyOption(char *space, char *var, char *value, int type, configNamespace *head);

/* Converters */
int Option2Int(char *space, char *var, configNamespace *head);
long Option2Long(char *space, char *var, configNamespace *head);
float Option2Float(char *space, char *var, configNamespace *head);
double Option2Double(char *space, char *var, configNamespace *head);
char* Option2String(char *space, char *var, configNamespace *head);

#endif

