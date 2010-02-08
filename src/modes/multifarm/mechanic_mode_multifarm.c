#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_multifarm.h"

int mechanic_mode_multifarm(int node, void* handler, moduleInfo* md, configData* d){


  if(node == 0) mechanic_mode_multifarm_master(node, handler, md, d);
  if(node != 0) mechanic_mode_multifarm_slave(node, handler, md, d);

  return 0;
}
