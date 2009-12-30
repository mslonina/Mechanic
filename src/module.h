#ifndef MODULE_H
#define MODULE_H

#define ELEMENTS 20

typedef struct inputData_t {
  char name[256];
  char datafile[260];
  int dump;
  int xres;
  int yres;
  int method;
} inputData_t;

inputData_t d;

/*
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
*/
#endif
