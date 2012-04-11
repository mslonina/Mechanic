/**
 * @file
 * The setup stage
 */
#include "MSetup.h"

/**
 * @function
 * The Setup
 *
 * @brief
 * This function reads the configuration file on the master node and broadcasts it to all
 * MPI_COMM_WORLD.
 *
 * @param int
 *  The node id
 * @param char*
 *  The filename to read
 * @param module*
 *  The module used, must be allocated and checked before
 * @param int
 *  The runtime mode
 *
 * @return
 *  The error code, 0 otherwise
 */
int Setup(module *m, char *filename, int argc, char** argv, int mode) {
  int mstat = 0, opts = 0, i = 0, n = 0;
  MPI_Datatype mpi_t;
  MPI_Status mpi_status;
  hid_t h5location;

  /* Read the specified configuration file */
  if (m->node == MASTER) {
    if (mode == RESTART_MODE) {
      h5location = H5Fopen(m->filename, H5F_ACC_RDONLY, H5P_DEFAULT);
      LRC_HDF5Parser(h5location, CONFIG_GROUP, m->layer.setup.head);
      H5Fclose(h5location);
    } else {
      mstat = ReadConfig(m, filename, m->layer.setup.head);
    }
  }

  LRC_head2struct_noalloc(m->layer.setup.head, m->layer.setup.options);
  opts = LRC_allOptions(m->layer.setup.head);

  /* In case of config file, we need to update the popt tables with new defaults  */
  PoptOptions(m, &m->layer.setup);

  /* Read popt options and overwrite the config file */
  mstat = Popt(m, argc, argv, &m->layer.setup);
  if (mstat != 0) return mstat;

  if (mode != RESTART_MODE) {
    LRC_head2struct_noalloc(m->layer.setup.head, m->layer.setup.options);
  }

  /* Broadcast new configuration */
  mstat = LRC_datatype(m->layer.setup.options[0], &mpi_t);

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
  LRC_cleanup(m->layer.setup.head);

  m->layer.setup.head = LRC_assignDefaults(m->layer.setup.options);
  if (m->node == MASTER && LRC_option2int("core", "print-defaults", m->layer.setup.head)) {
    LRC_printAll(m->layer.setup.head);
  }

  return mstat;
}

/**
 * @function
 * Reads the configuration file
 */
int ReadConfig(module *m, char *filename, LRC_configNamespace *head) {
  int mstat = 0;
  FILE *inif;

  if (filename == NULL || filename[0] == LRC_NULL) {
    Message(MESSAGE_ERR, "Option -c|--config specified but no valid config file present\n");
    Error(CORE_ERR_SETUP);
  }

  inif = fopen(filename, "ro");
  if (inif) {
    mstat = LRC_ASCIIParser(inif, SEPARATOR, COMMENTS, head);
    fclose(inif);
  } else if (inif == NULL) {
    if (strcmp(filename, LRC_getOptionValue("core", "config", m->layer.setup.head)) != 0) {
      Message(MESSAGE_ERR, "The specified config file could not be opened\n");
      Error(CORE_ERR_SETUP);
    }
    Message(MESSAGE_INFO, "No config file specified, using defaults.\n");
  }

  return mstat;
}

/**
 * @function
 * Popt command line parser
 */
int Popt(module *m, int argc, char** argv, setup *s) {
  int rc, mstat = 0;

  /* Get the command line args */
  s->popt->poptcontext = poptGetContext(NULL, argc, (const char **) argv, s->popt->popt, 0);
  rc = poptGetNextOpt(s->popt->poptcontext);
  if (rc < -1) {
    if (m->node == MASTER) {
      Message(MESSAGE_WARN,"%s: %s\n", poptBadOption(s->popt->poptcontext, POPT_BADOPTION_NOALIAS),
          poptStrerror(rc));
      poptPrintHelp(s->popt->poptcontext, stdout, 0);
    }
    return CORE_SETUP_HELP;
  }

  /* Update the LRC options struct with values from command line */
  LRCUpdate(s);

  if (LRC_option2int("core", "help", s->head)) {
    if (m->node == MASTER) {
      poptPrintHelp(s->popt->poptcontext, stdout, 0);
    }
    return CORE_SETUP_HELP;
  }

  if (LRC_option2int("core", "usage", s->head)) {
    if (m->node == MASTER) {
      poptPrintUsage(s->popt->poptcontext, stdout, 0);
    }
    return CORE_SETUP_HELP;
  }

  return mstat;
}

/**
 * @function
 * Initialize the popt tables according to the Setup()
 */
int PoptOptions(module *m, setup *s) {
  int mstat = 0, i = 0;
  char* garbage;

  /* Module options */
  while (s->options[i].name[0] != LRC_NULL) {
    if (s->options[i].type == LRC_STRING) {
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_STRING,
          &s->popt->string_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
    if (s->options[i].type == LRC_INT) {
      s->popt->int_args[i] = (int)strtol(s->options[i].value, &garbage, 0);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_INT,
          &s->popt->int_args[i], 0, s->options[i].description, s->options[i].value
      };
    }
   if (s->options[i].type == LRC_VAL) {
      s->popt->int_args[i] = (int)strtol(s->options[i].value, &garbage, 0);
      s->popt->popt[i] = (struct poptOption) {
        s->options[i].name, s->options[i].shortName, POPT_ARG_VAL,
          &s->popt->int_args[i], 1, s->options[i].description, NULL
      };
    }
    if (s->options[i].type == LRC_DOUBLE) {
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
 * @function
 * Updatesthe LRC options tablewith popt values
 */
int LRCUpdate(setup *s) {
  int i = 0, mstat = 0;
  size_t len;

  while (s->options[i].name[0] != LRC_NULL) {
    if (s->options[i].type == LRC_STRING) {
      if (s->popt->string_args[i] != NULL) {
        if (LRC_trim(&s->popt->string_args[i][0]) != LRC_NULL) {
          len = strlen(LRC_trim(s->popt->string_args[i]));
          strncpy(s->options[i].value, LRC_trim(s->popt->string_args[i]), len);
          s->options[i].value[len] = LRC_NULL;
        }
      }
    }
    if (s->options[i].type == LRC_INT) {
      sprintf(s->options[i].value,"%d",s->popt->int_args[i]);
    }
    if (s->options[i].type == LRC_VAL) {
      sprintf(s->options[i].value,"%d",s->popt->int_args[i]);
    }
    if (s->options[i].type == LRC_DOUBLE) {
      sprintf(s->options[i].value,"%f",s->popt->double_args[i]);
    }
    LRC_modifyOption(s->options[i].space, s->options[i].name,
        s->options[i].value, s->options[i].type, s->head);
    i++;
  }
  return mstat;
}
