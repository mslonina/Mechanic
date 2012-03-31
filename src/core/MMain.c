/**
 * C programmers never die. They are just cast into void.
 * 
 * Dennis Ritchie (1941-2011)
 */
 
/**
 * @file
 * The Main function
 */
#include "MMain.h"

int main(int argc, char** argv) {
  
  int mpi_rank, mpi_size, node;
  module core, module, fallback;

  char *filename;

  int mstat = 0;

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  node = mpi_rank;

  /* Initialize HDF */
  mstat = H5open();
  CheckStatus(mstat);

  if (node == MASTER) Welcome();

  /* Bootstrap */
  fallback.layer.handler = NULL;
  core = Bootstrap(node, mpi_size, CORE_MODULE, &fallback);
  if (!core.layer.handler) Error(CORE_ERR_CORE); // OOPS! At least Core module should be loaded

  /**
   * @todo
   * If module is not present (i.e. test drive), bootstrap core the second time with the
   * fallback of core
   *
   * @todo
   * replace TEST_MODULE with popt
   */
  module = Bootstrap(node, mpi_size, TEST_MODULE, &core);
  if (node == MASTER) 
    Message(MESSAGE_INFO, "Module '%s' bootstrapped.\n", TEST_MODULE);

  /* Setup */
  filename = Name("mechanic-", "config", "", ".cfg");

  mstat = Setup(&module, filename, NORMAL_MODE);
  CheckStatus(mstat);

  free(filename);

  /**
   * @todo Popt
   */ 
//  Popt(node, argc, argv, &module.layer.setup);
//  poptPrintHelp(module.layer.setup.poptcontext, stdout, 0);

  /* Modes */
  mstat = Taskfarm(&module);
  CheckStatus(mstat);

  MPI_Barrier(MPI_COMM_WORLD);

  ModuleFinalize(&module);
  ModuleFinalize(&core);
  
  H5close();
  MPI_Finalize();

  if (node == MASTER)
    Message(MESSAGE_INFO, "Mechanic did the job. Have a nice day!\n");
}
