#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_multifarm.h"

int mechanic_mode_multifarm_slave(int node, void* handler, moduleInfo* md, configData* d){

  printf("Multifarm SLAVE[%d]\n", node);
  return 0;
}
