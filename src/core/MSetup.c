/**
 * @file
 * The setup stage
 */
#include "MSetup.h"

/**
 * @brief The Setup
 *
 * This function reads the configuration on the master node and broadcasts it to all
 * MPI_COMM_WORLD. Use the CORE_SETUP for core, and MODULE_SETUP for module configuration. 
 *
 * @param m The module pointer
 * @param filename The filename to read
 * @param argc The command line argc table
 * @param argv The command line argv table
 * @param setup_mode One of the flags: CORE_SETUP, MODULE_SETUP
 *
 * @return 0 on success, error code otherwise
 */
int Setup(module *m, char *filename, int argc, char** argv, int setup_mode) {
  int mstat = SUCCESS, opts = 0, i = 0, n = 0;
  MPI_Datatype mpi_t;
  MPI_Status mpi_status;
  hid_t h5location;

  /* Read the specified configuration file */
  if (m->node == MASTER) {
    if (m->mode == RESTART_MODE) {
      h5location = H5Fopen(m->filename, H5F_ACC_RDONLY, H5P_DEFAULT);
      H5CheckStatus(h5location);

      if (setup_mode == CORE_SETUP) {
        ConfigHDF5Parser(h5location, "/config/core", m->layer.setup.head);
      }

      if (setup_mode == MODULE_SETUP) {
        ConfigHDF5Parser(h5location, "/config/module", m->layer.setup.head);
      }

      H5Fclose(h5location);
    } else {
      if (setup_mode == MODULE_SETUP) {
        mstat = ReadConfig(m, filename, m->layer.setup.head, setup_mode);
        CheckStatus(mstat);
      }
    }
  }

  ConfigHead2StructNoalloc(m->layer.setup.head, m->layer.setup.options);
  opts = ConfigAllOptions(m->layer.setup.head);

  /* In case of config file, we need to update the popt tables with new defaults  */
  PoptOptions(m, &m->layer.setup);

  /* Read popt options and overwrite the config file */
  mstat = Popt(m, argc, argv, &m->layer.setup, setup_mode);
  if (mstat != 0) return mstat;

  if (m->mode != RESTART_MODE) {
    ConfigHead2StructNoalloc(m->layer.setup.head, m->layer.setup.options);
  }

  /* Broadcast new configuration */
  mstat = ConfigDatatype(m->layer.setup.options[0], &mpi_t);
  CheckStatus(mstat);

  for (i = 0; i < opts; i++) {
    if (m->node == MASTER) {
      for (n = 1; n < m->mpi_size; n++) {
        MPI_Send(&m->layer.setup.options[i], 1, mpi_t, n, TAG_STANDBY, MPI_COMM_WORLD);
      }
    } else {
      MPI_Recv(&m->layer.setup.options[i], 1, mpi_t, MASTER, TAG_STANDBY, MPI_COMM_WORLD, &mpi_status);
    }
  }
  MPI_Type_free(&mpi_t);

  /**
   * Cleanup previous default configuration
   * Reassigning defaults should be faster than modifying options one-by-one
   */
  ConfigCleanup(m->layer.setup.head);

  m->layer.setup.head = ConfigAssignDefaults(m->layer.setup.options);
  if (setup_mode == MODULE_SETUP
      && m->node == MASTER
      && Option2Int("core", "print-defaults", m->layer.setup.head)) {
    ConfigPrintAll(m->layer.setup.head);
  }

  return mstat;
}

/**
 * @brief Reads the configuration file
 *
 * @param m The module pointer
 * @param filename The name of the configuration file
 * @param head The LRC linked list pointer
 * @param setup_mode One of the flags: CORE_SETUP, MODULE_SETUP
 *
 * @return 0 on success, error code otherwise
 */
int ReadConfig(module *m, char *filename, configNamespace *head, int setup_mode) {
  int mstat = SUCCESS;
  FILE *inif;

  if (filename == NULL || filename[0] == CONFIG_NULL) {
    Message(MESSAGE_ERR, "Option -c|--config specified but no valid configuration file present\n");
    Error(CORE_ERR_SETUP);
  }


  inif = fopen(filename, "ro");
  if (inif) {
    if (setup_mode == MODULE_SETUP) {
      Message(MESSAGE_INFO, "Configuration file to use: '%s'\n", filename);
      mstat = ConfigAsciiParser(inif, SEPARATOR, COMMENTS, head);
      CheckStatus(mstat);
    }
    fclose(inif);
  } else if (inif == NULL) {
    if (strcmp(filename, Option2String("core", "config", m->layer.setup.head)) != 0) {
      Message(MESSAGE_ERR, "The specified configuration file could not be opened\n");
      Error(CORE_ERR_SETUP);
    }
    if (setup_mode == MODULE_SETUP)
      Message(MESSAGE_INFO, "No configuration file specified, using defaults\n");
  }

  return mstat;
}

/**
 * @brief Popt command line parser
 *
 * @param m The module pointer
 * @param argc The command line argc table
 * @param argv The command line argv table
 * @param s The setup pointer
 * @param setup_mode The setup mode (CORE_SETUP, MODULE_SETUP)
 *
 * Note:
 * Since Popt tables does not hold the option namespace, options must have unique name
 * (i.e. it is not possible to have same options in different LRC namespaces)
 *
 * @return 0 on success, error code otherwise
 */
int Popt(module *m, int argc, char** argv, setup *s, int setup_mode) {
  int rc, mstat = SUCCESS;

  /* Get the command line args */
  s->popt->poptcontext = poptGetContext(NULL, argc, (const char **) argv, s->popt->popt, 0);
  rc = poptGetNextOpt(s->popt->poptcontext);
  if (rc < -1) {
    if (m->node == MASTER && setup_mode == MODULE_SETUP) {
      Message(MESSAGE_WARN,"%s: %s\n", poptBadOption(s->popt->poptcontext, POPT_BADOPTION_NOALIAS),
          poptStrerror(rc));
    }
    return CORE_SETUP_HELP;
  }

  /* Update the LRC options struct with values from command line */
  ConfigUpdate(s);

  if (Option2Int("core", "help", s->head)) {
    return CORE_SETUP_HELP;
  }

  if (Option2Int("core", "usage", s->head)) {
    return CORE_SETUP_USAGE;
  }

  if (Option2Int("core", "blocking", s->head)) {
    m->communication_type = MPI_BLOCKING;
  }

  return mstat;
}

/**
 * @brief Initialize the popt tables according to the Setup()
 *
 * @param m The module pointer
 * @param s The setup pointer
 *
 * @return 0 on success, error code otherwise
 */
int PoptOptions(module *m, setup *s) {
  int mstat = SUCCESS, i = 0;
  char* garbage;

  /* Module options */
  while (s->options[i].name[0] != CONFIG_NULL) {
    if (s->options[i].type == C_STRING) {
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_STRING,
          &s->popt->string_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
    if (s->options[i].type == C_VAL) {
      s->popt->val_args[i] = (int)strtol(s->options[i].value, &garbage, 0);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_VAL,
          &s->popt->val_args[i], 1, s->options[i].description, NULL
       };
     }
    if (s->options[i].type == C_INT) {
      s->popt->int_args[i] = (int)strtol(s->options[i].value, &garbage, 0);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_INT,
          &s->popt->int_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
    if (s->options[i].type == C_LONG) {
      s->popt->long_args[i] = strtol(s->options[i].value, &garbage, 0);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_LONG,
          &s->popt->long_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
    if (s->options[i].type == C_FLOAT) {
      s->popt->float_args[i] = strtof(s->options[i].value, &garbage);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_FLOAT,
          &s->popt->float_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
    if (s->options[i].type == C_DOUBLE) {
      s->popt->double_args[i] = strtod(s->options[i].value, &garbage);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_DOUBLE,
          &s->popt->double_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
    i++;
  }

  s->popt->popt[i+1] = (struct poptOption) POPT_TABLEEND;

  return mstat;
}

/**
 * @brief Updates the LRC options table with popt values
 *
 * @param s The setup pointer
 *
 * @return 0 on success, error code otherwise
 */
int ConfigUpdate(setup *s) {
  int i = 0, mstat = SUCCESS;
  size_t len;

  while (s->options[i].name[0] != CONFIG_NULL) {
    if (s->options[i].type == C_STRING) {
      if (s->popt->string_args[i] != NULL) {
        if (ConfigTrim(&s->popt->string_args[i][0]) != CONFIG_NULL) {
          len = strlen(ConfigTrim(s->popt->string_args[i]));
          strncpy(s->options[i].value, ConfigTrim(s->popt->string_args[i]), len);
          s->options[i].value[len] = CONFIG_NULL;
        }
      }
    }
    
    if (s->options[i].type == C_VAL) {
      sprintf(s->options[i].value, "%d", s->popt->val_args[i]);
    }
    if (s->options[i].type == C_INT) {
      sprintf(s->options[i].value, "%d", s->popt->int_args[i]);
    }
    if (s->options[i].type == C_LONG) {
      sprintf(s->options[i].value, "%ld", s->popt->long_args[i]);
    }
    if (s->options[i].type == C_FLOAT) {
      sprintf(s->options[i].value, "%.19E", s->popt->float_args[i]);
    }
    if (s->options[i].type == C_DOUBLE) {
      sprintf(s->options[i].value, "%.19E", s->popt->double_args[i]);
    }

    ModifyOption(s->options[i].space, s->options[i].name,
        s->options[i].value, s->options[i].type, s->head);
    i++;
  }

  return mstat;
}

