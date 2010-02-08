#ifndef MECHANIC_MODULE_ECHO_H
#define MECHANIC_MODULE_ECHO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <dirent.h>
#include <math.h>
#include <popt.h>
#include <dlfcn.h>

#include "mpi.h"
#include "hdf5.h"

#include "libreadconfig.h"


/**
 * Here You can define internals of Your modules.
 * Remember -- each of defined variable becomes global
 *
 */

#define ELEMENTS 20

struct slaveData{
  int test;
  MECHANIC_DATATYPE points[ELEMENTS];
};

struct slaveData sd;


struct slaveData *makeSlaveData(void){
  struct slaveData *pointer = malloc(sizeof(struct slaveData));
  if(pointer == NULL)
    return NULL;

  int i = 0;

  for(i = 0; i < ELEMENTS; i++){
    pointer->points[i] = 0.0;  
  }
  pointer->test = 0;
  return pointer;
};

/*
void initSlaveDataPointer(struct slaveData *pointer){
  pointer = malloc(sizeof(struct slaveData));
  if(pointer == NULL)
    return NULL;

  int i = 0;

  for(i = 0; i < ELEMENTS; i++){
    pointer->points[i] = 0.0;  
  }
  pointer->test = 23;
//  return pointer;
}
*/

#endif
