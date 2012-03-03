/**
 * @file
 */
#include "MMain.h"

int main(int argc, char** argv) {
  
  int mpirank, mpisize, node;
  module core, module, fallback;

  char *filename;

  int mstat = 0;

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
  node = mpirank;

  /* Popt */

  /* Bootstrap */
bootstrap:
  fallback.layer.handler = NULL;
  core = Bootstrap(node, CORE_MODULE, &fallback);
  if (!core.layer.handler) Error(CORE_ERR_CORE); // OOPS! At least Core module should be loaded

  /**
   * @todo
   * If module is not present (i.e. test drive), bootstrap core the second time with the
   * fallback of core
   *
   * @todo
   * replace TEST_MODULE with popt
   */
  module = Bootstrap(node, TEST_MODULE, &core);

  /* Setup */
setup:
  filename = Filename("mechanic-", "config", "", ".cfg");
  mstat = Setup(node, &module, filename, NORMAL_MODE);
  free(filename);


finalize:
  ModuleFinalize(node, &module);
  ModuleFinalize(node, &core);
  
  MPI_Finalize();
}
