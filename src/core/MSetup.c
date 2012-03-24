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
int Setup(module *m, char *filename, int mode) {
  int mstat = 0, opts = 0, i = 0, n = 0;
  MPI_Datatype mpi_t;
  MPI_Status mpi_status;
  char *fname;
  hid_t h5location;

  /* Read the specified configuration file */
  if (m->node == MASTER) {
    if (mode == RESTART_MODE) {
      h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
      LRC_HDF5Parser(h5location, CONFIG_GROUP, m->layer.setup.head);
      H5Fclose(h5location);
    } else {
      mstat = ReadConfig(filename, m->layer.setup.head);
    }
  }

  LRC_head2struct_noalloc(m->layer.setup.head, m->layer.setup.options);
  opts = LRC_allOptions(m->layer.setup.head);

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

  /**
   * Write the configuration to the master file
   */
  if (m->node == MASTER && mode != RESTART_MODE) {
    fname = Name(LRC_getOptionValue("core", "name", m->layer.setup.head),
      "-master", "-00", ".h5");

    strncpy(m->filename, fname, strlen(fname));
    m->filename[strlen(fname)] = LRC_NULL;
    
    h5location = H5Fcreate(m->filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    LRC_HDF5Writer(h5location, CONFIG_GROUP, m->layer.setup.head);
    H5Fclose(h5location);
    free(fname);
  }

  return mstat;
}

/**
 * @function
 * Reads the configuration file
 */
int ReadConfig(char *filename, LRC_configNamespace *head) {
  int mstat = 0;
  FILE *inif;

  inif = fopen(filename, "ro");
  if (inif) {
    mstat = LRC_ASCIIParser(inif, SEPARATOR, COMMENTS, head);
  } else {
    Message(MESSAGE_ERR, "Error opening config file");
    Error(MESSAGE_ERR);
  }
  fclose(inif);

  return mstat;
}

/**
 * @function
 * Popt command line parser
 */
int Popt(int node, int argc, char** argv, setup *s) {
  //poptContext context;
  char value;
  s->poptcontext = poptGetContext(NULL, argc, (const char **) argv, s->popt, 0);
  value = poptGetNextOpt(s->poptcontext);

  //printf("value = %s\n", value);

  return 0;
}

int PoptCountOptions(struct poptOption *p) {
  int options = 0;
  while (p[options].longName != NULL) {
    printf("longName = %s\n", p[options].longName);
    options++;
  }
  return options;
}

int PoptMergeOptions(module *m, struct poptOption *in, struct poptOption *add) {
  int index, addopts, i;
  int status = 0;
  
  index = PoptCountOptions(in);
  addopts = PoptCountOptions(add);

  for (i = 0; i < addopts; i++) {
    in[index+i] = add[i];
//    if (m->fallback.handler) {
//      in[index+i].arg = m->fallback.layer.setup.options[i].value;
//    }
  }

  return status;

}
