#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_farm.h"

int mechanic_mode_farm(int node, void* handler, moduleInfo* md, configData* d){

  if(node == 0) mechanic_mode_farm_master(node, handler, md, d);
  if(node != 0) mechanic_mode_farm_slave(node, handler, md, d);

  return 0;
}
