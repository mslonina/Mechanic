#include <mechanic/mechanic.h>
#include "mechanic_module_hello.h"

/**
 * Sample module for MpiFarm
 * Keep is simple, but powerful!
 */

/* Required */
void hello_module_init(){
  return;
}

/* Required */
void hello_module_cleanup(){
  return;
}

/* Required */
void hello_pixelCompute(int slave, configData *d, masterData *r){

  r->res[0] = (double)r->coords[0];
  r->res[1] = (double)r->coords[1];
  r->res[2] = (double)r->coords[2];
  return;
}

/**
 * Not required, but we want to show how useful it can be:)
 */
void hello_slaveOUT(int slave, configData *d, masterData *r){
  printf("Hello from slave[%d]\n", slave);
  return;
}
