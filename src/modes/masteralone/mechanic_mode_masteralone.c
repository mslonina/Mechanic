#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_masteralone.h"

int mechanic_mode_masteralone(int node, void* handler, moduleInfo* md, configData* d){

  int i = 0;
  
  if(node == 0){
    printf("I will compute everything by self, I promise! \n");
  }

#if HAVE_MPI_SUPPORT
  if(node == 0){
  for(i = 1; i < mpi_size; i++){
    printf("Terminating SLAVE[%d]\n",i);
    MPI_Send(map2d(1, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
  }
}
#endif

  return 0;
}
