/**
 * @file
 * Runtime configuration
 */
#include "M2Cprivate.h"

/**
 * @brief Prints error messages
 * 
 * @param j The line in the config file where the error exist
 * @param type Type of the error
 * @param message Error message to print
 */
void ConfigMessage(int line, int type, char *message) {
  switch (type) {
    case CONFIG_ERR_CONFIG_SYNTAX:
      Message(MESSAGE_ERR, "%s at line %d: %s\n", CONFIG_MSG_CONFIG_SYNTAX, line, message);
      break;
    case CONFIG_ERR_FILE_OPEN:
      Message(MESSAGE_ERR, "%s at line %d: %s\n", CONFIG_MSG_FILE_OPEN, line, message);
      break;
    default:
      break;
  }
}

/**
 * @brief Remove leading, trailing, & excess embedded spaces
 *
 * Public domain by Bob Stout
 * @see http://www.cs.umu.se/~isak/snippets/trim.c
 *
 * @param str String to trim.
 */
char* ConfigTrim(char *str) {
  char *ibuf = str, *obuf = str;
  int i = 0, cnt = 0;

  /* Trap NULL */
  if (str != NULL) {
 
    /* Remove leading spaces (from RMLEAD.C) */
    for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf)
      ;
    if (str != ibuf)
      memmove(str, ibuf, ibuf - str);

    /* Collapse embedded spaces (from LV1WS.C) */
    while (*ibuf) {
      if (isspace(*ibuf) && cnt) ibuf++;
        else{
          if (!isspace(*ibuf)) cnt = 0;
            else {
              *ibuf = ' ';
              cnt = 1;
              }
            obuf[i++] = *ibuf++;
            }
          }
          obuf[i] = CONFIG_NULL;

     /* Remove trailing spaces (from RMTRAIL.C) */
     while (--i >= 0) {if (!isspace(obuf[i])) break;}
     obuf[++i] = CONFIG_NULL;
    }

  return str;
}

/**
 * @brief Check namespace name and trim it
 *
 * @param l Name of the namespace
 *
 * @return Trimmed name of the namespace
 */
char* ConfigNameTrim(char *l) {
  int len = 0;

  len = strlen(l);

  /* Quick and dirty solution using trim function */
  l[0] = ' ';
  l[len-1] = ' ';
  l = ConfigTrim(l);  

  return l;
}

/**
 * @brief Count the number of separators in the current line
 *
 * @param l Current line
 * @param s The separator to count
 *
 * @return Number of separators in the current line
 */
int ConfigCharCount(char *l, char *s) {
  int i = 0, sep = 0, len = 0; 

  len = strlen(l);

  for (i = 0; i < len; i++) {
    if ((int) *s == l[i]) sep++;
  }
  
  return sep;
}

/**
 * @brief Helper function for creating new namespaces
 */
configNamespace* ConfigNewNamespace(char *cfg) {
  configNamespace *newNM = NULL;

  newNM = calloc(sizeof(configNamespace), sizeof(configNamespace));
  if (!newNM) {
    Message(MESSAGE_ERR, "ConfigNewNamespace: allocation failed.");
    Error(CORE_ERR_MEM);
  }

  strncpy(newNM->space, cfg, strlen(cfg));
  newNM->space[strlen(cfg)] = CONFIG_NULL;
  newNM->options = NULL;
  newNM->next = NULL;

  return newNM;
}

/**
 *  ASCII parser
 *
 *  @brief Reads config file, namespaces, variable names and values,
 *  into the options structure @see configNamespace.
 *
 *  @param read Handler of the config file to read
 *  @param sep The separator name/value
 *  @param comm The comment mark
 *  @param head Pointer to the structure with datatypes allowed in the config file
 *
 *  @return Number of namespaces found in the config file on success, -1 otherwise
 */
int ConfigAsciiParser(FILE *read, char *sep, char *comm, configNamespace *head) {
  int j = 0, sepc = 0, n = 0;
  char *line, l[CONFIG_MAX_LINE_LENGTH], *b, *c, *value; 

  config *newOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  size_t valuelen = 0;

  if (!head) {
    Message(MESSAGE_ERR, "ConfigAsciiParser: No config assigned");
    return -1;
  }

  current = head;

  while (!feof(read)) {
    
    /* Count lines */
    j++; 
  
    line = fgets(l, CONFIG_MAX_LINE_LENGTH, read);
    
    /* Skip blank lines and any NULL */
    if (line == NULL) break;
    if (line[0] == '\n') continue;
    
    /* Now we have to trim leading and trailing spaces etc */
    line = ConfigTrim(line);

    /* Check for full line comments and skip them */
    if (strspn(line, comm) > 0) continue;
    
    /* Check for the separator at the beginning */
    if (strspn(line, sep) > 0) {
      ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_MISSING_VAR); 
      goto failure;
    }

    /* First split var/value and inline comments.
     * Trim leading and trailing spaces */
    b = ConfigTrim(strtok(line, comm));

    /* Check for namespaces */
    if (b[0] == '[') {
      if (b[strlen(b)-1] != ']') {
        ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_MISSING_BRACKET); 
        goto failure;
      }

      b = ConfigNameTrim(b);
			
			nextNM = ConfigFindNamespace(b, head);
			
			if (nextNM == NULL) {
				ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_UNKNOWN_NAMESPACE);
				goto failure;
			} else {
				current = nextNM;
			}
      
      n++;

      continue;
    }
  
    /* If no namespace was specified return failure */
    if (current == NULL) {
      ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_NONAMESPACE);
      goto failure;
    }

    /* Check if in the var/value string the separator exist.*/
    if (strstr(b, sep) == NULL) {
      ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_MISSING_SEP); 
      goto failure;
    }
    
    /* Check some special case:
     * we have separator, but no value */
    if ((strlen(b) - 1) == strcspn(b, sep)) {
      ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_MISSING_VAL); 
      goto failure;
    }

    /* We allow to have only one separator in line */
    sepc = ConfigCharCount(b, sep);
    if (sepc > 1) {
      ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_TOOMANY_SEP); 
      goto failure;
    }
    
    /* Ok, now we are prepared */
    c = ConfigTrim(strtok(b, sep));

		newOP = ConfigFindOption(c, current);
		
		if (newOP == NULL) {
      ConfigMessage(j, CONFIG_ERR_CONFIG_SYNTAX, CONFIG_MSG_UNKNOWN_VAR); 
      goto failure;
		}

    while (c != NULL) {
      if (c[0] == '\n') break;

      valuelen = strlen(c);
			value = calloc(valuelen + sizeof(char), sizeof(char));
      if (!value) {
        Message(MESSAGE_ERR, "ConfigAsciiParser: allocation failed");
        goto failure;
      }
			strncpy(value, c, valuelen);
			value[valuelen] = CONFIG_NULL;

			strncpy(newOP->value, value, valuelen);
			newOP->value[valuelen] = CONFIG_NULL;
			
			c = ConfigTrim(strtok(NULL, "\n"));

			if (value) free(value);
    }  
		
  }
  return n;

failure:
  return -1;
}

/**
 * @brief Cleanup assign pointers. This is required for proper memory managment
 */
void ConfigCleanup(configNamespace *head) {
  config *currentOP = NULL;
  config *nextOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  if (head) {
    current = head;

    while (current) {
      nextNM = current->next;
      currentOP = current->options;
        
      if (currentOP) {
        while (currentOP) {
          nextOP = currentOP->next;
          if (currentOP) free(currentOP);
          currentOP = nextOP;
        }
      }

      if (current) free(current);
      current=nextNM;
    }

    head = NULL;
  }
}

/**
 * @brief Write ASCII config file
 *
 * @return 0 on success, errcode otherwise
 */
int ConfigAsciiWriter(FILE *write, char *sep, char *comm, configNamespace *head) {
  config *currentOP = NULL;
  config *nextOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  if (!head) {
    Message(MESSAGE_ERR, "ConfigAsciiWriter: no config assigned");
    return -1;
  }

  current = head;

  fprintf(write, "%s Written by Mechanic Config \n", comm);

  do {
    if (current) {
      fprintf(write, "[%s]\n", current->space);
      currentOP = current->options; 
      do {
        if (currentOP) {
          fprintf(write, "%s %s %s\n", currentOP->name, sep, currentOP->value);
          nextOP = currentOP->next;
          currentOP = nextOP;
        }
      } while(currentOP);
      
      nextNM = current->next;
      current = nextNM;
    }
    fprintf(write,"\n");
  } while(current);

  fprintf(write,"\n");
  return 0;
}

/**
 * @brief Prints all options
 */
void ConfigPrintAll(configNamespace *head) {
  config *currentOP = NULL;
  config *nextOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  if (head) current = head;

  do {
    if (current) {
      nextNM = current->next;
      Message(MESSAGE_OUTPUT, "[%s][%d]\n", current->space, ConfigCountOptions(current->space, current));
      currentOP = current->options;
      do {
        if (currentOP) {
          nextOP = currentOP->next;
          Message(MESSAGE_OUTPUT, "%s = %s [type %d]\n", currentOP->name, currentOP->value, currentOP->type);
          currentOP = nextOP;
        }
      } while(currentOP);
      
      current=nextNM;
    }
		printf("\n");
    
  } while(current);

  current = NULL;

}

/**
 * @brief Assign default values
 *
 * @return 0 on success, errcode otherwise
 */
configNamespace* ConfigAssignDefaults(options *cd) {
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;
  configNamespace *head = NULL;
  config *newOP = NULL;
  config *nextOP = NULL;
  config *currentOP = NULL;
  size_t slen, nlen, vlen;
  char space[CONFIG_LEN]; 
  char name[CONFIG_LEN]; 
  char value[CONFIG_LEN]; 

  int i = 0;

  head = NULL;
  current = NULL;

  while (cd[i].space[0] != CONFIG_NULL) {

      /* Prepare namespace */
      slen = strlen(cd[i].space);
      strncpy(space,cd[i].space, slen);
			space[slen] = CONFIG_NULL;

      if (head == NULL) {
        head = ConfigNewNamespace(space);
        current = head;
      } else {
        nextNM = ConfigFindNamespace(space, head);

        if (nextNM == NULL) {
          current = ConfigLastLeaf(head);
          current->next = ConfigNewNamespace(space);
          current = current->next;
        } else {
				  current = nextNM;
          current->next = NULL;
			  }
      }

      if (current) { 

				/* Prepare var name*/
       	nlen = strlen(cd[i].name);
       	strncpy(name, cd[i].name, nlen);
				name[nlen] = CONFIG_NULL;

				currentOP = ConfigFindOption(name, current);

        if (currentOP == NULL) {  	
					newOP = calloc(sizeof(config), sizeof(config));
          if (!newOP) {
            Message(MESSAGE_ERR, "ConfigAssignDefaults: allocation failed");
            Error(CORE_ERR_MEM);
          }
          newOP->type = C_INT;
          newOP->next = NULL;
				
				  if (current->options == NULL) {
         	  current->options = newOP;
            current->options->next = NULL;
					  currentOP = current->options;
				  } else {
					  currentOP = current->options;
            nextOP = currentOP->next;
					  if (nextOP) {
              do {
                if (!nextOP->next) currentOP = nextOP;
                nextOP = nextOP->next;
              } while (nextOP);
					  }
					  currentOP->next = newOP;
					  currentOP = newOP;
				  }
				
       	  strncpy(currentOP->name, name, nlen);
				  currentOP->name[nlen] = CONFIG_NULL;
				
          /* Prepare the value */
          vlen = strlen(cd[i].value);
          strncpy(value, cd[i].value, vlen);
          value[vlen] = CONFIG_NULL;
            
          strncpy(currentOP->value, value, vlen);
          currentOP->value[vlen] = CONFIG_NULL;
        
          /* Assign type */
          currentOP->type = cd[i].type;
				} else {
          ModifyOption(current->space, currentOP->name, cd[i].value, cd[i].type, current);
        }
      }

    i++;
  }

  return head;
}

/**
 * @brief Count all available options
 *
 * @param head First namespace in the options list
 *
 * @return Number of options (all available namespaces)
 */
int ConfigAllOptions(configNamespace *head) {
  int allopts = 0;
  config *currentOP = NULL;
  config *nextOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  if (head) current = head;

  do {
    if (current) {
      nextNM = current->next;
      currentOP = current->options;
      do {
        if (currentOP) {
          allopts++;
          nextOP = currentOP->next;
          currentOP = nextOP;
        }
      } while(currentOP);
      current=nextNM;
    }
  } while(current);

  return allopts;
}

/**
 * @brief Search for given namespace
 *
 * @param namespace The name of the namespace to be searched for
 *
 * @return Pointer to the namespace or NULL if namespace was not found
 */
configNamespace* ConfigFindNamespace(char *namespace, configNamespace *head) {
  configNamespace *test = NULL;
  
  if (head && namespace) {
    test = head;

    while (test) {
      if (strcmp(test->space, namespace) == 0) {
        return test;
      }
      test = test->next;
    }
  }
  return NULL;
}

/**
 * @brief Get the last leaf of the linked list
 *
 * @param head The configuration namespace
 *
 * @return The last leaf on success, head pointer on failure
 */
configNamespace* ConfigLastLeaf(configNamespace *head) {
  configNamespace *test = NULL;

  if (head) {
    test = head;

    if (test->next == NULL) return test;

    while (test->next) {
      test = test->next;
      if (test->next == NULL) return test;
    }
  }

  return head;
}

/**
 * @brief Search for given variable
 *
 * @param varname The name of the variable to be searched for
 *
 * @return The pointer to the variable or NULL if the variable was not found
 *
 */
config* ConfigFindOption(char *varname, configNamespace *current) {
  config *testOP = NULL;
  size_t vlen, olen;
  char var[CONFIG_LEN];
  char opt[CONFIG_LEN];

  if (current && varname) {
    if (current->options) {
      testOP = current->options;
      vlen = strlen(varname);
      strncpy(var, varname, vlen);
      var[vlen] = CONFIG_NULL;

      while (testOP) {
        if (testOP->name) {

          olen = strlen(testOP->name);
          strncpy(opt, testOP->name, olen);
          opt[olen] = CONFIG_NULL;
          
          if (strcmp(opt, var) == 0) {
            return testOP;
          }
        }
        testOP = testOP->next;
      }
    }
  }

  return NULL;
}

/**
 * @brief Modifies value and type of given option
 *
 * @return The pointer to modified option or NULL if option was not found
 */
config* ModifyOption(char *namespace, char *varname, char *newvalue, int newtype, configNamespace *head) {
	config *option = NULL;
  configNamespace *current = NULL;
	size_t vlen;

  if (head) {
	  current = ConfigFindNamespace(namespace, head);
	  if (current) {
      option = ConfigFindOption(varname, current);

	    vlen = strlen(newvalue);

      if (option) {
        if (strcmp(option->value, newvalue) != 0) {
          strncpy(option->value, newvalue, vlen);
          option->value[vlen] = CONFIG_NULL;
        }
        if (option->type != newtype) {
          option->type = newtype;
        }
      }
    }
  }
	return option;
}

/**
 * @brief Converts the option to string
 *
 * @return Converted option
 */
char* Option2String(char *namespace, char *var, configNamespace *head) {
	config *option = NULL;
  configNamespace *current = NULL;

  if (head) {
    current = ConfigFindNamespace(namespace, head);
    if (current) {
      option = ConfigFindOption(var, current);
      if (option && option->value) return option->value;
    }
  }
  return NULL;
}

/**
 * @brief Converts the option to integer
 *
 * @return Converted option
 */
int Option2Int(char *namespace, char *varname, configNamespace *head) {
  config *option = NULL;
  configNamespace *current = NULL;
  int value = 0;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = atoi(option->value);
        }
      }
    }
  }

  return value;
}

/**
 * @brief Converts the option to long integer
 *
 * @return Converted option
 */
long Option2Long(char *namespace, char *varname, configNamespace *head) {
  config *option = NULL;
  configNamespace *current = NULL;
  long value = 0;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = atol(option->value);
        }
      }
    }
  }

  return value;
}

/**
 * @brief Converts the option to float
 *
 * @return Converted option
 */
float Option2Float(char *namespace, char *varname, configNamespace *head) {
  config *option = NULL;
  configNamespace *current = NULL;
  float value = 0.0;
  char *p = NULL;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = strtof(option->value, &p);
        }
      }
    }
  }
  
  return value;
}

/**
 * @brief Converts the option to double
 *
 * @return Converted option
 */
double Option2Double(char *namespace, char *varname, configNamespace *head) {
  config *option = NULL;
  configNamespace *current = NULL;
  double value = 0.0;
  char *p = NULL;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = strtod(option->value, &p);
        }
      }
    }
  }
  
  return value;
}

/**
 * @brief Counts number of options in given namespace
 *
 * @return Number of options in given namespace, 0 otherwise
 */
int ConfigCountOptions(char *nm, configNamespace *head) {
  configNamespace *nspace;
  config *option;
  int opts = 0;

  if (head && nm) {
    nspace = ConfigFindNamespace(nm, head);

    if (nspace) {
      option = nspace->options;
      while (option) {
        opts++;
        option = option->next;
      }
    }
  }

  return opts;
}

/**
 * @brief Counts the number of all options in the default Config options structure. The option
 * structure must end with OPTIONS_END
 *
 * @return Number of options
 */
int ConfigCountDefaultOptions(options *in) {
  int options;

  options = 0;

  while (in[options].space[0] != CONFIG_NULL) {
    options++;
  }

  return options;
}

/**
 * @brief Merges two Config default option structures
 *
 * @param options Input structure
 * @param options The structure, that we want to merge with the Input one
 *
 * @return Error code or 0 otherwise. The Input structure is extended with the second one.
 */
int ConfigMergeDefaults(options *in, options *add) {
  int status;
  int index, addopts, i, j, flag;
  status = 0;

  index = ConfigCountDefaultOptions(in);
  addopts = ConfigCountDefaultOptions(add);

  for (i = 0; i < addopts; i++) {
    flag = 0;
    for (j = 0; j < index; j++) {
      if (strcmp(in[j].name,add[i].name) == 0) {
        in[j] = add[i];
        flag = 1;
      } 
    }
    if (flag == 0) in[index + i] = add[i];
  }

  return status;
}

/**
 * @brief Counts all options in all namespaces
 *
 * @return The number of options
 */
int ConfigCountAllOptions(configNamespace *head) {
  int opts = 0, allopts = 0;
  configNamespace *current;

  current = head;
  while (current) {
    opts = ConfigCountOptions(current->space, current);
    allopts = allopts + opts;
    current = current->next;
  }
    
  return allopts;
}

/**
 * @brief Converts the linked list back to a structure without memory allocation
 *
 * @return The options structure
 */
int ConfigHead2StructNoalloc(configNamespace *head, options *c) {
  int i = 0;
  size_t len;

  config *currentOP = NULL;
  config *nextOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  if (head) {
    current = head;

    do { 
      if (current) {
        nextNM = current->next;
        currentOP = current->options;
        do { 
          if (currentOP) {
            len = strlen(current->space) + 1; 
            strncpy(c[i].space, current->space, len);
            len = strlen(currentOP->name) + 1; 
            strncpy(c[i].name, currentOP->name, len);
            len = strlen(currentOP->value) + 1; 
            strncpy(c[i].value, currentOP->value, len);
            c[i].type = currentOP->type;
            
            i++; 
            nextOP = currentOP->next;
            currentOP = nextOP;
          }    
        } while(currentOP);
        current = nextNM;
      }    
    } while(current);
  }

  return 0;
}

/**
 * @brief Converts the linked list back to a structure
 *
 * @return The option structure. You must free it
 */
options* ConfigHead2Struct(configNamespace *head) {
  options *c = NULL;
  int opts = 0;

  opts = ConfigAllOptions(head);

  c = calloc(opts * sizeof(options), sizeof(options));
  
  ConfigHead2StructNoalloc(head, c);
  return c;
}

