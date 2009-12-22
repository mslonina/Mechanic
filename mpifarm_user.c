#include "mpifarm.h"

/**
 * Output Data struct is defined as follows:
 * typedef struct{
 *   int count[3]; <-- this handles x,y coords and number of the pixel
 *   MY_DATATYPE res[MAX_RESULT_LENGTH]; <-- this handles result vector
 * }
 *
 */

/**
 * USER DEFINED FARM RESOLUTION
 *
 */
int userdefined_farmResolution(int x, int y){
  
  int farm;

  farm = x*y;

  return farm;
}

/**
 * USER DEFINED PIXEL CHOORDS
 * 
 */
void userdefined_pixelCoords(int slave, int t[],outputData *r){
          
  r->coords[0] = t[0];
  r->coords[1] = t[1];
  r->coords[2] = t[2];
  
  return;
}
void userdefined_pixelCoordsMap(int ind[], int p, int x, int y){
  
    if(p < y) ind[0] = p / y; ind[1] = p;
    if(p > y - 1) ind[0] = p / y; ind[1] = p % y;
  return;
}

/**
 * COMPUTE PIXEL
 * 
 */
void userdefined_pixelCompute(int slave, outputData *r){

   r->res[0] = (MY_DATATYPE) r->coords[0]; 
   r->res[1] = (MY_DATATYPE) r->coords[1];
   r->res[2] = r->res[0]*r->res[1];
   r->res[3] = 5.25;
   r->res[4] = 6.75;
   /* etc. */
  
   return;
}

/**
 * USER DEFINED MASTER_IN FUNCTION
 *
 */
void userdefined_masterIN(){
  return;
}

/**
 * USER DEFINED MASTER_OUT FUNCTION
 *
 */
void userdefined_masterOUT(){
    
  printf("Master process OVER & OUT.\n");
  
  return;
}

/**
 * USER DEFINED MASTER HELPER FUNCTIONS
 * called before/after send/receive
 * 
 */
void userdefined_master_beforeSend(int slave, outputData *r){
  return;
}
void userdefined_master_afterSend(int slave, outputData *r){
  return;
}
void userdefined_master_beforeReceive(outputData *r){
  return;
}
void userdefined_master_afterReceive(int slave, outputData *r){
  return;
}

/**
 * USER DEFINED SLAVE_IN FUNCTION
 *
 */
void userdefined_slaveIN(int slave){
  return;
}

/**
 * USER DEFINED SLAVE_OUT FUNCTION
 *
 */
void userdefined_slaveOUT(int slave){
  
  printf("SLAVE[%d] OVER & OUT\n",slave);

  return;
}

/**
 * USER DEFINED SLAVE HELPER FUNCTIONS
 * called before/after send/receive
 * 
 */
void userdefined_slave_beforeSend(int slave, outputData *r){
  
  printf("SLAVE[%d] working on pixel [ %d , %d ]: %f\n", slave, r->coords[0], r->coords[1], r->res[2]);
  
  return;
}
void userdefined_slave_afterSend(int slave, outputData *r){
  return;
}
void userdefined_slave_beforeReceive(int slave, outputData *r){
  return;
}
void userdefined_slave_afterReceive(int slave, outputData *r){
  return;
}


