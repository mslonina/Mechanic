#ifndef MPIFARM_MODULE_TTT_H
#define MPIFARM_MODULE_TTT_H

#undef MAX_RESULT_LENGTH
#define MAX_RESULT_LENGTH 30

#undef MY_DATATYPE
#define MY_DATATYPE double

#undef MY_MPI_DATATYPE
#define MY_MPI_DATATYPE MPI_DOUBLE

#undef ELEMENTS
#define ELEMENTS 20

#include <stdio.h>
#include <stdlib.h>

struct slaveData_t {
  int test;
  MY_DATATYPE points[ELEMENTS];
};

struct yourdata{
  int tt;
};

struct yourdata *makeyourdata(void){
 struct yourdata *pointer = malloc(sizeof(struct yourdata));
 if(pointer == NULL)
   return NULL;

 pointer->tt = 0;

 return pointer;
  
};

struct slaveData_t *makeSlaveData(void){
  struct slaveData_t *pointer = malloc(sizeof(struct slaveData_t));
  if(pointer == NULL)
    return NULL;

  int i = 0;

  for(i = 0; i < ELEMENTS; i++){
    pointer->points[i] = 0.0;  
  }
  pointer->test = 0;
  return pointer;
};

#endif
