/**
 * MPIFARM USER API
 *
 * Functions provided here are called during farm operations. 
 * We provide You with simple examples of using them -- 
 * the only limit is You imagination.
 *
 * Master Data struct is defined as follows:
 * typedef struct{
 *   int count[3]; <-- handles x,y coords and number of the pixel
 *   MY_DATATYPE res[MAX_RESULT_LENGTH]; <-- handles result vector
 * }
 *
 * Master Data is the only data received by master node,
 * however, You can do much with Slave Data struct -- You can redefine it in
 * mpifarm_user.h and use during simulation
 *
 * Input Data struct can be also redefined in mpifarm_user.h
 *
 */

#include "mpifarm.h"
#include "plugin.h"
#include <string.h>
#include "readconfig.h"
/*
void mpifarm_module_init(struct yourdata *pointer){
  printf("MODULE DEFAULT\n");
  pointer->tt = 1;
  return;
}

void mpifarm_module_cleanup(struct yourdata *pointer){
  return;
}*/

