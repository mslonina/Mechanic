/**
 * MPIFARM USER API
 *
 * Functions provided here are called during farm operations. 
 * We provide You with simple examples of using them -- 
 * the only limit is You imagination.
 *
 * Master Data struct is defined as follows:
 * typedef struct{
 *   int count[3]; <-- this handles x,y coords and number of the pixel
 *   MY_DATATYPE res[MAX_RESULT_LENGTH]; <-- this handles result vector
 * }
 *
 * Master Data is the only data received by master node,
 * however, You can do much with Slave Data struct -- You can redefine it in
 * mpifarm_user.h and use during simulation
 *
 * Input Data struct can be also redefined in mpifarm_user.h
 */

#include "mpifarm_skel.h"
#include <string.h>
#include "libreadconfig.h"

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
void userdefined_pixelCoords(int slave, int t[], inputData *d, masterData *r, slaveData *s){
          
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
 * USER DEFINED PIXEL COMPUTE
 * 
 */
void userdefined_pixelCompute(int slave, inputData *d, masterData *r, slaveData *s){

   r->res[0] = (MY_DATATYPE) r->coords[0]; 
   r->res[1] = (MY_DATATYPE) r->coords[1];
   r->res[2] = r->res[0]*r->res[1];
   r->res[3] = 5.25;
   r->res[4] = 6.75;
  
   return;
}

/**
 * USER DEFINED MASTER_IN FUNCTION
 * This function is called before any farm operations.
 *
 */
void userdefined_masterIN(int mpi_size, inputData *d){
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
void userdefined_masterOUT(int mpi_size, inputData *d, masterData *r){
  
  int i;
  hid_t fname, masterfile, masterdatagroup;
  herr_t stat;
  const char groupbase[] = "slave";
  char groupname[512];
  const char filebase[] = "slave";
  char filename[512];

  masterfile = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);
  masterdatagroup = H5Gopen(masterfile, DATAGROUP, H5P_DEFAULT);
  
  /**
   * Copy data from slaves to one master file
   */
  for(i = 1; i < mpi_size; i++){
    sprintf(groupname,"%s%d", groupbase,i);
    sprintf(filename,"%s-%s%d.h5", d->name, filebase,i);
   
    fname = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    stat = H5Ocopy(fname, groupname, masterdatagroup, groupname, H5P_DEFAULT, H5P_DEFAULT);
    H5Fclose(fname);

  }

  H5Gclose(masterdatagroup);
  H5Fclose(masterfile);
  printf("Master process OVER & OUT.\n");
  
  return;
}

/**
 * USER DEFINED MASTER HELPER FUNCTIONS
 * called before/after send/receive
 * 
 */
void userdefined_master_beforeSend(int slave, inputData *d, masterData *r){
  return;
}
void userdefined_master_afterSend(int slave, inputData *d, masterData *r){
  return;
}
void userdefined_master_beforeReceive(inputData *d, masterData *r){
  return;
}
void userdefined_master_afterReceive(int slave, inputData *d, masterData *r){
  return;
}

/**
 * USER DEFINED SLAVE_IN FUNCTION
 * 
 * Do some preparation here, i.e. 
 * -- clear proper arrays in slaveData s
 * -- read data to struct s, even from different files
 * -- create group/dataset for the slave etc.
 *
 * Example:
 * Here we create slave specific data file.
 * You can handle here any type of datasets etc.
 *
 * Data group is incorporated in MASTER_OUT function to one master data file.
 */
void userdefined_slaveIN(int slave, inputData *d, masterData *r, slaveData *s){

  clearArray(s->points, ITEMS_IN_ARRAY(s->points));

  hid_t sfile_id, sdatagroup, gid, string_type;
  hid_t dataset, dataspace;
  hid_t rank = 1;
  hsize_t dimens_1d;
  herr_t serr;
  const char sbase[] = "slave";
  char node[512];
  const char gbase[] = "slave";
  char group[512];

  const char cbase[] = "Hello from slave ";
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
  
  sprintf(comment, "%s%d.",cbase,slave); 
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
 * Example:
 * Just prints a message from the slave.
 */
void userdefined_slaveOUT(int slave, inputData *d, masterData *r, slaveData *s){
  
  printf("SLAVE[%d] OVER & OUT\n",slave);

  return;
}

/**
 * USER DEFINED SLAVE HELPER FUNCTIONS
 * called before/after send/receive
 * 
 */
void userdefined_slave_beforeSend(int slave, inputData *d, masterData *r, slaveData *s){
  
  printf("SLAVE[%d] working on pixel [ %d , %d ]: %f\n", slave, r->coords[0], r->coords[1], r->res[2]);
  
  return;
}
void userdefined_slave_afterSend(int slave, inputData *d, masterData *r, slaveData *s){
  return;
}
void userdefined_slave_beforeReceive(int slave, inputData *d, masterData *r, slaveData *s){
  return;
}
void userdefined_slave_afterReceive(int slave, inputData *d, masterData *r, slaveData *s){
  return;
}

/**
 * HELPER FUNCTION -- READ CONFIG VALUES
 * see libreadconfig.h for details
 *
 * Adjust the parser to Your initial data struct from mpifarm_user.h
 */
int userdefined_readConfigValues(char* inifile, char* sep, char* comm, inputData *d){

  int i = 0, k = 0, opts = 0, offset = 0;
  
  opts = parseConfigFile(inifile, sep, comm);

	for(i = 0; i < opts; i++){
    if(strcmp(configSpace[i].space,"default") == 0){
		  for(k = 0; k < configSpace[i].num; k++){
			  if(strcmp(configSpace[i].options[k].name,"name") == 0){
          strcpy(d->name,configSpace[i].options[k].value);
          strcpy(d->datafile, strcat(configSpace[i].options[k].value,"-master.h5"));  
        }
        if(strcmp(configSpace[i].options[k].name,"bodies") == 0) d->bodies = atoi(configSpace[i].options[k].value);
    }
    }
		if(strncmp(configSpace[i].space,"planet*",1) == 0){
		  for(k = 0; k < configSpace[i].num; k++){
        d->el[offset+k] = atof(configSpace[i].options[k].value);
		  }
      offset = offset + k;
		}
    if(strcmp(configSpace[i].space,"farm") == 0){
		  for(k = 0; k < configSpace[i].num; k++){
			  if(strcmp(configSpace[i].options[k].name,"xres") == 0) d->xres = atoi(configSpace[i].options[k].value);  
        if(strcmp(configSpace[i].options[k].name,"yres") == 0) d->yres = atoi(configSpace[i].options[k].value); 
			  if(strcmp(configSpace[i].options[k].name,"method") == 0) d->method = atoi(configSpace[i].options[k].value); 
      }
    }
	}

  return opts;
}

/**
 * USERDEFINED MPI BCAST
 * We send the only information needed by slaves
 */

void userdefined_mpiBcast(int mpi_rank, inputData *d){

  int buff_size = 1000;
  int *ibuff;
  float *fbuff;
  char *sbuff, *nbuff;

  ibuff = malloc(buff_size*sizeof(*ibuff));
  fbuff = malloc(buff_size*sizeof(*fbuff));
  sbuff = malloc(buff_size*sizeof(*sbuff));
  nbuff = malloc(buff_size*sizeof(*nbuff));
 
  if (mpi_rank == 0) {

    fbuff[0] = d->el[0];
    fbuff[1] = d->el[1];
    fbuff[2] = d->el[2];
    fbuff[3] = d->el[3];
    fbuff[4] = d->el[4];

    ibuff[0] = d->xres;
    ibuff[1] = d->yres;
    ibuff[2] = d->method;
    ibuff[3] = d->dump;
    ibuff[4] = d->bodies;

    strcpy(sbuff,d->datafile);
    strcpy(nbuff,d->name);

    MPI_Bcast (fbuff, buff_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast (ibuff, buff_size, MPI_INT,   0, MPI_COMM_WORLD);
    MPI_Bcast (sbuff, buff_size, MPI_CHAR,   0, MPI_COMM_WORLD);
    MPI_Bcast (nbuff, buff_size, MPI_CHAR,   0, MPI_COMM_WORLD);

  } else {

    MPI_Bcast (fbuff, buff_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast (ibuff, buff_size, MPI_INT,   0, MPI_COMM_WORLD);
    MPI_Bcast (sbuff, buff_size, MPI_CHAR,   0, MPI_COMM_WORLD);
    MPI_Bcast (nbuff, buff_size, MPI_CHAR,   0, MPI_COMM_WORLD);

    d->el[0]   = fbuff[0];
    d->el[1]   = fbuff[1];
    d->el[2]   = fbuff[2];
    d->el[3]   = fbuff[3];
    d->el[4]   = fbuff[4];

    d->xres     = ibuff[0];
    d->yres     = ibuff[1];
    d->method   = ibuff[2];
    d->dump = ibuff[3];
    d->bodies = ibuff[4];

    strcpy(d->datafile,sbuff);
    strcpy(d->name,nbuff);

  }
  free(ibuff);
  free(fbuff);
  free(sbuff);
  free(nbuff);
  
  return;
}
