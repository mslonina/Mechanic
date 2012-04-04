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
  char *module_name;

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

  /* Bootstrap core */
  fallback.layer.handler = NULL;
  core = Bootstrap(node, mpi_size, argc, argv, CORE_MODULE, &fallback);
  if (!core.layer.handler) Error(CORE_ERR_CORE); // OOPS! At least Core module should be loaded

  /* Fallback to core if module not specified */
  if (!core.popt->string_args[0]) {
    module_name = CORE_MODULE;  
  } else {
    module_name = core.popt->string_args[0];
  }

  /* Bootstrap the module */
  module = Bootstrap(node, mpi_size, argc, argv, module_name, &core);
  if (node == MASTER) 
    Message(MESSAGE_INFO, "Module '%s' bootstrapped.\n", module_name);

  /* Setup */
  if (!module.popt->string_args[1]) {
    filename = DEFAULT_CONFIG_FILE;
  } else {
    filename = module.popt->string_args[1];
  }

  if (node == MASTER)
    Message(MESSAGE_INFO, "Config file to use: '%s'\n", filename);

  mstat = Setup(&module, filename, argc, argv, NORMAL_MODE);

  if (mstat == CORE_SETUP_HELP) goto finalize; // Special help message handling
  CheckStatus(mstat);

  /* Modes */
  if (mpi_size > 1) {
    mstat = Taskfarm(&module);
    CheckStatus(mstat);
  } else {
    Message(MESSAGE_WARN, "Master-alone mode not supported yet. Aborting...\n");
  }
  
finalize:
  MPI_Barrier(MPI_COMM_WORLD);

  ModuleFinalize(&module);
  ModuleFinalize(&core);
  
  H5close();
  MPI_Finalize();

  if (node == MASTER)
    Message(MESSAGE_INFO, "Mechanic did the job. Have a nice day!\n");

  return EXIT_SUCCESS;
}
