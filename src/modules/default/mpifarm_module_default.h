#ifndef MPIFARM_MODULE_DEFAULT_H
#define MPIFARM_MODULE_DEFAULT_H

/**
 * Here You can define internals of Your modules.
 * Remember -- each of defined variable becomes global
 *
 */

#define ELEMENTS 20

struct slaveData{
  int test;
  MY_DATATYPE points[ELEMENTS];
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
