#include <mechanic/mechanic.h>
#include "mechanic_module_hello.h"

/**
 * Sample module for MpiFarm
 * Keep is simple, but powerful!
 */

/* Required */
int hello_init(moduleInfo* md){
  md->author = "John Smith";
  md->date = "2010";
  md->version = "1.0";
  md->mrl = 3;
  return 0;
}

/* Required */
int hello_cleanup(moduleinfo* md){
  return 0;
}

/* Required */
int hello_pixelCompute(int slave, moduleInfo* md, configData* d, masterData* r){

  r->res[0] = (double)r->coords[0];
  r->res[1] = (double)r->coords[1];
  r->res[2] = (double)r->coords[2];
  return 0;
}

/**
 * Not required, but we want to show how useful it can be:)
 */
int hello_slaveOUT(int slave, moduleInfo* md, configData* d, masterData* r){
  printf("Hello from slave[%d]\n", slave);
  return 0;
}
