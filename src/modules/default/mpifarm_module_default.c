/**
 * MPIFARM PLUGIN DEFAULT
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
#include "mpifarm_module_default.h"

/*
void mpifarm_module_init(struct yourdata *pointer){
  //printf("Module DEFAULT INIT\n");
  //pointer->aa = 1.1;
  //pointer->bb = 2.2;
  return;
}

void mpifarm_module_query(struct yourdata *pointer){
  //printf("Module DEFAULT QUERY\n");
  return;
}

void mpifarm_module_cleanup(struct yourdata *pointer){
  //printf("Module DEFAULT CLEANUP: %f\n", pointer->aa + pointer->bb);

  free(pointer);
  return;
}
*/
void default_init(moduleInfo *md){

  md->name = "default";
  md->author = "MSlonina";
  md->date = "2010";
  md->version = "1.0";

//  sd.test = 21;
  return;
}
void default_query(){
//  printf("TEST = %d\n", s->test);
  return;
}
void default_cleanup(){
//  printf("SD.TEST = %d\n", sd.test);
  return;
}

/**
 * USER DEFINED FARM RESOLUTION
 *
 * Returns farm resolution
 *
 */
int default_farmResolution(int x, int y){
  
  int farm;

  farm = x*y;

  return farm;
}

/**
 * USER DEFINED PIXEL COORDS MAP
 *
 * t[] is sent to each slave.
 * You can overwrite default pixel coords alignment here.
 *
 * Used only when method = 6.
 * 
 */
void default_pixelCoordsMap(int t[], int p, int x, int y){
  
    if(p < y) t[0] = p / y; t[1] = p;
    if(p > y - 1) t[0] = p / y; t[1] = p % y;
  return;
}

/**
 * USER DEFINED PIXEL COORDS
 *
 * Each slaves takes the pixel coordinates and then do its work.
 * Here You can change pixel assignment to output masterData r.
 *
 * Used only when method = 6.
 *
 */
void default_pixelCoords(int slave, int t[], configData* d, masterData* r){
          
  r->coords[0] = t[0]; //x 
  r->coords[1] = t[1]; //y
  r->coords[2] = t[2]; //number of the pixel
  
  return;
}

/**
 * USER DEFINED PIXEL COMPUTE
 * 
 * The heart. Here You can compute your pixels.
 *
 * Example:
 * We assign some values to the result array of masterData r and 
 * do some weird computations.
 * Size of the array is controlled by MAX_RESULT_LENGTH from mpifarm_user.h 
 *
 */
void default_pixelCompute(int slave, configData* d, masterData* r){

  int i = 0;

   //  printf("D->MRL = %d\n",d->mrl);

   for(i = 0; i < d->mrl; i++){
      r->res[i] = pow(sin(i), 2.0) + pow(cos(i), 2.0) + pow(r->coords[0], 8.0) - pow(r->coords[1], 7.0);
   }
  
   return;
}

/**
 * USER DEFINED MASTER_IN FUNCTION
 * 
 * This function is called before any farm operations.
 *
 */
void default_masterIN(int mpi_size, configData* d){
  return;
}

/**
 * USER DEFINED MASTER_OUT FUNCTION
 *
 * This function is called after all operations are performed.
 * 
 * Example:
 * Here, we just copy slave data files into one master file
 *
 */
void default_masterOUT(int nodes, configData* d, masterData* r){
  
  int i = 0;
  hid_t fname, masterfile, masterdatagroup;
  herr_t stat;
  char groupname[512];
  char filename[512];

  //printf("masterfile: %s\n", d->datafile);

  stat = H5open();
  masterfile = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);
  masterdatagroup = H5Gopen(masterfile, DATAGROUP, H5P_DEFAULT);
  
  /**
   * Copy data from slaves to one master file
   */
  for(i = 1; i < nodes; i++){
    sprintf(groupname,"slave%d", i);
    sprintf(filename,"%s-slave%d.h5", d->name,i);
   
    //printf("filename[%d]: %s, groupname[%d]: %s\n", i, filename, i, groupname);
    fname = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    stat = H5Ocopy(fname, groupname, masterdatagroup, groupname, H5P_DEFAULT, H5P_DEFAULT);
    if(stat < 0) printf("copy error\n");
    H5Fclose(fname);

  }

  H5Gclose(masterdatagroup);
  H5Fclose(masterfile);
  stat = H5close();
  
  printf("Master process OVER & OUT.\n");
  return;
}

/**
 * USER DEFINED MASTER HELPER FUNCTIONS
 * 
 * Called before/after send/receive
 * 
 */
void default_master_beforeSend(int slave, configData* d, masterData* r){
  return;
}
void default_master_afterSend(int slave, configData* d, masterData* r){
  return;
}
void default_master_beforeReceive(configData* d, masterData* r){
  return;
}
void default_master_afterReceive(int slave, configData* d, masterData* r){
  return;
}

/**
 * USER DEFINED SLAVE_IN FUNCTION
 * 
 * Called before slave starts its work.
 *
 * Do some preparation here, i.e. 
 * -- clear proper arrays in slaveData_t s
 * -- read data to struct s, even from different files
 * -- create group/dataset for the slave etc.
 *
 * Example:
 * Here we create slave specific data file.
 * You can handle here any type of datasets etc.
 *
 * Data group is incorporated in MASTER_OUT function to one master data file.
 *
 */
void default_slaveIN(int slave, configData* d, masterData* r){

  //clearArray(s->points, ITEMS_IN_ARRAY(s->points));

  hid_t sfile_id, sdatagroup, gid, string_type;
  hid_t dataset, dataspace;
  hid_t rank = 1;
  hsize_t dimens_1d;
  herr_t serr;
  char sbase[] = "slave";
  char node[512];
  char gbase[] = "slave";
  char group[512];

  char cbase[] = "Hello from slave ";
  char comment[1024];

  sprintf(node, "%s-%s%d.h5", d->name, sbase, slave);
  sprintf(group, "%s%d", gbase, slave);

  /**
   * Imagine this:
   * each slave can create different dataspaces and datasets here, 
   * perform different computations, even read different config file!
   */
  sfile_id = H5Fcreate(node, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  gid = H5Gcreate(sfile_id, group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
 
  sprintf(comment, "%s%d. ",cbase,slave); 
  string_type = H5Tcopy(H5T_C_S1);
  H5Tset_size(string_type, strlen(comment));

  rank = 1;
  dimens_1d = 1;

  dataspace = H5Screate_simple(rank, &dimens_1d, NULL);
  
  dataset = H5Dcreate(gid, "comment", string_type, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  serr = H5Dwrite(dataset, string_type, H5S_ALL, dataspace, H5P_DEFAULT, comment);

  H5Sclose(dataspace);
  H5Dclose(dataset);
  H5Gclose(gid);
  H5Fclose(sfile_id);

   return;
}

/**
 * USER DEFINED SLAVE_OUT FUNCTION
 *
 * Called after slave done its work.
 *
 * Example:
 * Just prints a message from the slave.
 */
void default_slaveOUT(int slave, configData* d, masterData* r){
  
  printf("SLAVE[%d] OVER & OUT\n",slave);

  return;
}

/**
 * USER DEFINED SLAVE HELPER FUNCTIONS
 * 
 * Called before/after send/receive
 * 
 */
void default_slave_beforeSend(int slave, configData* d, masterData* r){
  
  printf("SLAVE[%d] working on pixel [ %d , %d ]: %f\n", slave, r->coords[0], r->coords[1], r->res[4]);
  
  return;
}
void default_slave_afterSend(int slave, configData* d, masterData* r){
  return;
}
void default_slave_beforeReceive(int slave, configData* d, masterData* r){
  return;
}
void default_slave_afterReceive(int slave, configData* d, masterData* r){
  return;
}

