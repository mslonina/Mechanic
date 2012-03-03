/**
 * @file
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
int Setup(int node, module *m, char *filename, int mode) {
  int mstat = 0, opts = 0, i = 0, n = 0;
  MPI_Datatype mpi_t;
  int mpisize;
  MPI_Status mpi_status;
  
  if (node == MASTER) {
    mstat = ReadConfig(filename, m->layer.setup.head);
  }

  LRC_head2struct_noalloc(m->layer.setup.head, m->layer.setup.options);
  opts = LRC_allOptions(m->layer.setup.head);

  /* Broadcast new configuration */
  mstat = LRC_datatype(m->layer.setup.options[0], &mpi_t);

  for (i = 0; i < opts; i++) {
    if (node == MASTER) {
      MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
      for (n = 1; n < mpisize; n++) {
        MPI_Send(&m->layer.setup.options[i], 1, mpi_t, n, STANDBY, MPI_COMM_WORLD);
      }
    } else {
      MPI_Recv(&m->layer.setup.options[i], 1, mpi_t, MASTER, STANDBY, MPI_COMM_WORLD, &mpi_status);
    }
  }
  MPI_Type_free(&mpi_t);

  /**
   * Cleanup previous default configuration 
   * Reassigning defaults should be faster than modifying options one-by-one
   */
  LRC_cleanup(m->layer.setup.head);
  m->layer.setup.head = LRC_assignDefaults(m->layer.setup.options);

  LRC_printAll(m->layer.setup.head);

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
  }
  fclose(inif);

  return mstat;
}

