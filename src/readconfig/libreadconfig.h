#ifndef LIBREADCONFIG_H
#define LIBREADCONFIG_H

#define MAX_LINE_LENGTH 1024
#define MAX_CONFIG_SIZE 1024
#define MAX_NAME_LENGTH 256
#define MAX_VALUE_LENGTH 256
#define MAX_OPTIONS_NUM 50

#define E_CONFIG_SYNTAX "Config file syntax error."
#define E_MISSING_VAR "Missing variable name."
#define E_MISSING_VAL "Missing value."
#define E_MISSING_SEP "Missing separator."
#define E_MISSING_BRACKET "Missing bracket in namespace."
#define E_TOOMANY_SEP "To many separators."

/**
 * define options structs
 */
typedef struct configOptions {
  char name[MAX_NAME_LENGTH];
  char value[MAX_VALUE_LENGTH];
} configOptions;

typedef struct configNamespace {
  char space[MAX_NAME_LENGTH]; //name of the namespace
  configOptions options[MAX_OPTIONS_NUM]; //options for given namespace
  int num; //number of options read
} configNamespace;

void configError(int, char*);
char* trim(char*);
char* nametrim(char*, int);
int charcount(char*, char*);
int parsefile(FILE*, char*, char*, configNamespace*);
void printAll(int, configNamespace*);
int parseConfigFile(char*, char*, char*, configNamespace*);

#endif
