/**
 * @file
 */
#include "MMain.h"

int main(int argc, char** argv) {
  
  int mpirank, mpisize, node, mstat;
  LRC_configNamespace* opthead;
  layer core, module;
  query *q;

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
  node = mpirank;

bootstrap:
  core = Bootstrap(node, CORE_MODULE);
  if (!core.handler) Error(CORE_ERR_CORE); // OOPS! At least Core module should be loaded

  module = Bootstrap(node, TEST_MODULE);

  if (node == MASTER && core.setup.head) LRC_printAll(core.setup.head);
  if (node == MASTER && module.setup.head) LRC_printAll(module.setup.head);

finalize:
  Finalize(node, &module);
  Finalize(node, &core);
  
  MPI_Finalize();
}
