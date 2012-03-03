/**
 * @file
 */
#include "MMain.h"

int main(int argc, char** argv) {
  
  int mpirank, mpisize, node;
  module core, module, fallback;

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
  node = mpirank;

bootstrap:
  fallback.layer.handler = NULL;
  core = Bootstrap(node, CORE_MODULE, &fallback);
  if (!core.layer.handler) Error(CORE_ERR_CORE); // OOPS! At least Core module should be loaded

  /**
   * @todo
   * If module is not present (i.e. test drive), bootstrap core the second time with the
   * fallback of core
   */
  module = Bootstrap(node, TEST_MODULE, &core);

  if (node == MASTER && core.layer.setup.head) LRC_printAll(core.layer.setup.head);
  if (node == MASTER && module.layer.setup.head) LRC_printAll(module.layer.setup.head);

  /* Setup Broadcast */

finalize:
  Finalize(node, &module);
  Finalize(node, &core);
  
  MPI_Finalize();
}
