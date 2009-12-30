/* MPI FARM SKELETON
 * Task farm model
 *
 * by mariusz slonina <mariusz.slonina@gmail.com>
 * with a little help of kacper kowalik <xarthisius.kk@gmail.com>
 *
 * BE CEAREFUL -- almost no error handling (TODO)
 */
#include "mpifarm.h"
#include <string.h>

static int* map2d(int, void *);
static void master(lt_dlhandle, int);
static void slave(lt_dlhandle, int);
static void master_test(lt_dlhandle);
static void slave_test(lt_dlhandle);
void clearArray(MY_DATATYPE*,int);

/* Simplified struct for writing config data to hdf file */
typedef struct {
      char space[MAX_NAME_LENGTH];
      char varname[MAX_NAME_LENGTH];
      char value[MAX_VALUE_LENGTH];
    } simpleopts;

char* sep = "="; char* comm = "#";
char* inifile;
char* datafile;

/* settings */ 
int allopts = 0; //number of options read

/* farm */ 
int xres = 0, yres = 0, method = 0, datasetx = 1, datasety = 3;

/* mpi */ 
MPI_Status mpi_status;
int mpi_rank, mpi_size, mpi_dest = 0, mpi_tag = 0;
int farm_res = 0;
int npxc = 0; //number of all sent pixels
int count = 0; //number of pixels to receive
int source_tag = 0, data_tag = 2, result_tag = 59, terminate_tag = 99; //data tags

int masteralone = 0;
int nodes = 0;

unsigned int membersize, maxsize;
int position, msgsize;
char *buffer;

int i = 0, j = 0, k = 0, opts = 0, n = 0;

/* hdf */
hid_t file_id, dset_board, dset_data, dset_config, data_group;
hid_t configspace, configmemspace, mapspace, memmapspace, rawspace, memrawspace, maprawspace; 
hid_t plist_id;
hsize_t dimsf[2], dimsr[2], co[2], rco[2], off[2], stride[2];
int fdata[1][1];
herr_t hdf_status;
hid_t hdf_config_datatype;

/**
 * MAIN
 *
 */
int main(int argc, char *argv[]){  

  char *module_name;
  char module_file[256];
  lt_dlhandle module;
  char *dlresult;
  char optvalue;

  void *prestartmode;
  int restartmode = 0;
  int poptflags = 0;
  int error;

  int mrl;
  mrl = 15;

  module_query_int_f qconfig;
  module_query_void_f qbcast;

  /*  Default config and module file */
  inifile = CONFIG_FILE_DEFAULT;
  module_name = MODULE_DEFAULT;

  struct poptOption cmdopts[] = {
    POPT_AUTOHELP
    {"restart", 'r', POPT_ARG_NONE, &prestartmode, 'r',
    "enable restart mode [TODO]","RESTART"},
    {"config", 'c', POPT_ARG_STRING || POPT_ARGFLAG_SHOW_DEFAULT, &inifile, 'c',
    "change config file", "CONFIG"},
    {"module", 'm', POPT_ARG_STRING || POPT_ARGFLAG_SHOW_DEFAULT, &module_name, 'm',
    "provide the module", "MODULE"},
    POPT_TABLEEND
  };

  lt_dlinit();
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  poptContext poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, POPT_CONTEXT_POSIXMEHARDER);

  if(argc < 2){
        poptPrintHelp(poptcon, stderr, poptflags);
        MPI_Finalize();
        return 0;
  }
  
  while((optvalue = poptGetNextOpt(poptcon)) >= 0){
    switch (optvalue){
      case 'c':
        break;
      case 'm':
        break;
      case '?':
        poptPrintHelp(poptcon, stderr, poptflags);
      case 'r':
        poptPrintHelp(poptcon, stderr, poptflags);
        MPI_Finalize();
        return 0;
      default:
        break;
    }
  }
  if (optvalue < -1){
    fprintf(stderr, "%s: %s\n",
        poptBadOption(poptcon, POPT_BADOPTION_NOALIAS), 
        poptStrerror(optvalue));
        MPI_Finalize();
        return 1;
  }

  /**
   * Plugin loading, if option -m is not set, use default
   */
  sprintf(module_file, "mpifarm_module_%s.so", module_name);
  
  module = lt_dlopen(module_file);
  if(!module){
    printf("Cannot load module '%s': %s\n", module_name, lt_dlerror()); 
    MPI_Abort(MPI_COMM_WORLD, 913);
  }

  init = lt_dlsym(module, "mpifarm_module_init");

  /* test only */
  struct yourdata *pointer;
  pointer = makeyourdata();
  
  init(pointer);

  query = lt_dlsym(module, "mpifarm_module_query");
  query(pointer);
 
  /**
   * Each slave knows exactly what is all about
   * -- it is much easier to handle
   */
  if(mpi_rank == 0){
    qconfig = lt_dlsym(module, "userdefined_readConfigValues");
    allopts = qconfig(inifile, sep, comm, &d); 
   
    if (d.xres == 0 || d.yres == 0){
       printf("X/Y map resolution should not set to 0!\n");
       printf("If You want to do only one simulation, please set xres = 1, yres = 1\n");
       MPI_Abort(MPI_COMM_WORLD, 911);
    }
    qbcast = lt_dlsym(module, "userdefined_mpiBcast");
    qbcast(mpi_rank,&d);
  }else{
    qbcast = lt_dlsym(module, "userdefined_mpiBcast");
    qbcast(mpi_rank,&d);
  }

  /**
   * Create data pack
   */
  MPI_Pack_size(3, MPI_INT, MPI_COMM_WORLD, &membersize);
  maxsize = membersize;
  MPI_Pack_size(mrl, MY_MPI_DATATYPE, MPI_COMM_WORLD, &membersize);
  maxsize += membersize;
  buffer = malloc(maxsize);
      
  /**
   * BY DESIGN
   * Master cannot work alone.
   */
  if (mpi_size == 1){
     printf("You should have at least one slave!\n");
     masteralone = 1;

     MPI_Abort(MPI_COMM_WORLD, 911);
  }

  if(mpi_rank == 0) {
      master(module, mrl);
  //    master_test(module);
	} else {
      slave(module, mrl); 
//      slave_test(module);
  }

  //printf("cleanup lt_dlsym\n");
  cleanup = lt_dlsym(module, "mpifarm_module_cleanup");

  //printf("cleanup \n");
  cleanup(pointer);
  
  //printf("module close\n");
 // lt_dlerror();
  lt_dlclose(module);
 //   printf("Closing module '%s': %s\n", module_name, lt_dlerror()); 

  //printf("poptFree\n");
  poptFreeContext(poptcon);

  //printf("free buffer\n");
  free(buffer);

  //printf("mpi final\n");
	MPI_Finalize();
	lt_dlexit();

  return 0;
}

/**
 * MASTER
 *
 */
static void master(lt_dlhandle module, int mrl){

   int *tab; 
	 MY_DATATYPE rdata[mrl][1];
   int i = 0, k = 0, j = 0;
   
   module_query_void_f queryIN, queryOUT, qbeforeS, qafterS, qbeforeR, qafterR;
   module_query_int_f qr;
  
   masterData raw;
   masterData *rawdata;
 
  /**
    * Allocate memory for rawdata.res array
    */
   rawdata = malloc(sizeof(rawdata) + (mrl-1) * sizeof(MY_DATATYPE));

  // clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
   
   printf("MAP RESOLUTION = %dx%d.\n", d.xres, d.yres);
   printf("COMM_SIZE = %d.\n", mpi_size);
   printf("METHOD = %d.\n", d.method);

   /**
    * Master can do something useful before computations
    */
   query = lt_dlsym(module, "userdefined_masterIN");
   query(mpi_size, &d);

   /**
    * Align farm resolution for given method
    */

   if (method == 0) farm_res = d.xres*d.yres; //one pixel per each slave
   if (method == 1) farm_res = d.xres; //sliceX
   if (method == 2) farm_res = d.yres; //sliceY
   if (method == 6){
     qr = lt_dlsym(module, "userdefined_farmResolution");
     farm_res = qr(d.xres, d.yres);
   }

   hsize_t dim[1];
   unsigned rr = 1;
 
   /**
    * HDF5 Storage
    *
    * We write data in the following scheme:
    * /config -- configuration file
    * /board -- map of computed pixels
    * /data -- output data group
    * /data/master -- master dataset
    *
    * Each slave can write own data files if needed.
    * In such case, please edit slaveIN/OUT functions in mpifarm_user.c.
    *
    */

   /* Create master datafile */
    file_id = H5Fcreate(d.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    
    /* Number of options to write */
    j = 0;
    for(i = 0; i < allopts; i++){
      for(k = 0; k < configSpace[i].num; k++){
        j++;
      }
    }

    dim[0] = j;

    /* Define simplified struct */
    simpleopts optstohdf[j];
    
    /* Copy config file to our simplified struct */
    j = 0;
      for(i = 0; i < allopts; i++){
        for(k = 0; k < configSpace[i].num; k++){
          strcpy(optstohdf[j].space,configSpace[i].space);
          strcpy(optstohdf[j].varname,configSpace[i].options[k].name);
          strcpy(optstohdf[j].value,configSpace[i].options[k].value);
          j++;
        }
      }

    /* Config dataspace */
    configspace = H5Screate_simple(rr, dim, NULL);

    /* Create compound data type for handling config struct */
    hid_t hdf_optsarr_dt = H5Tcreate(H5T_COMPOUND, sizeof(simpleopts));
    
      hid_t space_dt = H5Tcopy(H5T_C_S1);
      H5Tset_size(space_dt, MAX_NAME_LENGTH);
    
      hid_t varname_dt = H5Tcopy(H5T_C_S1);
      H5Tset_size(varname_dt, MAX_NAME_LENGTH);
    
      hid_t value_dt = H5Tcopy(H5T_C_S1);
      H5Tset_size(value_dt, MAX_NAME_LENGTH);

    H5Tinsert(hdf_optsarr_dt, "Namespace", HOFFSET(simpleopts, space), space_dt);
    H5Tinsert(hdf_optsarr_dt, "Variable", HOFFSET(simpleopts, varname), varname_dt);
    H5Tinsert(hdf_optsarr_dt, "Value", HOFFSET(simpleopts, value), value_dt);

    dset_config = H5Dcreate(file_id, DATASETCONFIG, hdf_optsarr_dt, configspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    configmemspace = H5Screate_simple(rr, dim, NULL);

    hdf_status = H5Dwrite(dset_config, hdf_optsarr_dt, H5S_ALL, H5S_ALL, H5P_DEFAULT, optstohdf);

    H5Tclose(hdf_optsarr_dt);
    H5Dclose(dset_config);
    H5Sclose(configspace);
    H5Sclose(configmemspace);
    
    /* Control board space */
    dimsf[0] = d.xres;
    dimsf[1] = d.yres;
    mapspace = H5Screate_simple(HDF_RANK, dimsf, NULL);
    
    dset_board = H5Dcreate(file_id, DATABOARD, H5T_NATIVE_INT, mapspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* Master data group */
    data_group = H5Gcreate(file_id, DATAGROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
    /* Result data space  */
    dimsr[0] = d.xres*d.yres;
    dimsr[1] = mrl;
    rawspace = H5Screate_simple(HDF_RANK, dimsr, NULL);

    /* Create master dataset */
    dset_data = H5Dcreate(data_group, DATASETMASTER, H5T_NATIVE_DOUBLE, rawspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  
   /**
    * We write pixels one by one
    */
    co[0] = 1;
    co[1] = 1;
    memmapspace = H5Screate_simple(HDF_RANK, co, NULL);

    rco[0] = 1;
    rco[1] = mrl;
    memrawspace = H5Screate_simple(HDF_RANK, rco, NULL);
   
   /**
    * Select hyperslab in the file
    */
    mapspace = H5Dget_space(dset_board);
    rawspace = H5Dget_space(dset_data);
  
   /**
    * FARM STARTS
    */

   /**
    * Security check -- if farm resolution is greater than number of slaves.
    * Needed when we have i.e. 3 slaves and only one pixel to compute.
    */
   if (farm_res > mpi_size) nodes = mpi_size;
   if (farm_res == mpi_size) nodes = mpi_size;
   if (farm_res < mpi_size) nodes = farm_res + 1;

   /**
    * Send tasks to all slaves
    */
   for (i = 1; i < nodes; i++){
     qbeforeS = lt_dlsym(module,"userdefined_master_beforeSend");
     qbeforeS(i,&d,rawdata);

     MPI_Send(map2d(npxc, module), 3, MPI_INT, i, data_tag, MPI_COMM_WORLD);
     
     qafterS = lt_dlsym(module, "userdefined_master_afterSend");
     qafterS(i,&d,rawdata);
     
     count++;
     npxc++;
   }

   /**
    * Receive data and send tasks
    */
   while (1) {
    
     qbeforeR = lt_dlsym(module, "userdefined_master_beforeReceive");
     qbeforeR(&d,rawdata);
      
     MPI_Recv(buffer, maxsize, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
     count--;
      
      /**
       * Unpack data sent by the slave
       */
      position = 0;
      MPI_Get_count(&mpi_status, MPI_PACKED, &msgsize);
      MPI_Unpack(buffer, msgsize, &position, rawdata->coords, 3, MPI_INT, MPI_COMM_WORLD);
      MPI_Unpack(buffer, msgsize, &position, rawdata->res, mrl, MY_MPI_DATATYPE, MPI_COMM_WORLD);
     
      qafterR = lt_dlsym(module, "userdefined_master_afterReceive");
      qafterR(mpi_status.MPI_SOURCE, &d, rawdata);

      /**
       * Write results to master file
       */
      
      /**
       * Control board -- each computed pixel is marked with 1
       */
      off[0] = rawdata->coords[0];
      off[1] = rawdata->coords[1];
      fdata[0][0] = 1;
      H5Sselect_hyperslab(mapspace, H5S_SELECT_SET, off, NULL, co, NULL);
      hdf_status = H5Dwrite(dset_board, H5T_NATIVE_INT, memmapspace, mapspace, H5P_DEFAULT, fdata);

      /**
       * Data
       */
      off[0] = rawdata->coords[2];
      off[1] = 0;
      for (j = 0; j < mrl; j++){
        rdata[j][0] = rawdata->res[j];
      }
      
      H5Sselect_hyperslab(rawspace, H5S_SELECT_SET, off, NULL, rco, NULL);
      hdf_status = H5Dwrite(dset_data, H5T_NATIVE_DOUBLE, memrawspace, rawspace, H5P_DEFAULT, rdata);

      /**
       * Send next task to the slave
       */
      if (npxc < farm_res){
       
        qbeforeS = lt_dlsym(module, "userdefined_master_beforeSend");
        qbeforeS(mpi_status.MPI_SOURCE, &d, rawdata);
         
        MPI_Send(map2d(npxc, module), 3, MPI_INT, mpi_status.MPI_SOURCE, data_tag, MPI_COMM_WORLD);
        npxc++;
        count++;
        
        qafterS = lt_dlsym(module, "userdefined_master_afterSend");
        qafterS(mpi_status.MPI_SOURCE, &d, rawdata);
      } else {
        break;
      }
    }

    /** 
     * No more work to do, receive the outstanding results from the slaves 
     * We've got exactly 'count' messages to receive 
     */
    for (i = 0; i < count; i++){
      
      qbeforeR = lt_dlsym(module, "userdefined_master_beforeReceive");
      qbeforeR(&d, rawdata);
        
      MPI_Recv(buffer, maxsize, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      position = 0;
      MPI_Get_count(&mpi_status, MPI_PACKED, &msgsize);
      MPI_Unpack(buffer, msgsize, &position, rawdata->coords, 3, MPI_INT, MPI_COMM_WORLD);
      MPI_Unpack(buffer, msgsize, &position, rawdata->res, mrl, MY_MPI_DATATYPE, MPI_COMM_WORLD);
      
      qafterR = lt_dlsym(module, "userdefined_master_afterReceive");
      qafterR(mpi_status.MPI_SOURCE, &d, rawdata);
        
        /**
         * Write outstanding results to file
         */
      
        /**
         * Control board
         */
        off[0] = rawdata->coords[0];
        off[1] = rawdata->coords[1];
        
        fdata[0][0] = 1;
        H5Sselect_hyperslab(mapspace, H5S_SELECT_SET, off, NULL, co, NULL);
        hdf_status = H5Dwrite(dset_board, H5T_NATIVE_INT, memmapspace, mapspace, H5P_DEFAULT, fdata);
        
        /**
         * Data
         */
        off[0] = rawdata->coords[2];
        off[1] = 0;
        
        for (j = 0; j < mrl; j++){
          rdata[j][0] = rawdata->res[j];
        }
      
        H5Sselect_hyperslab(rawspace, H5S_SELECT_SET, off, NULL, rco, NULL);
        hdf_status = H5Dwrite(dset_data, H5T_NATIVE_DOUBLE, memrawspace, rawspace, H5P_DEFAULT, rdata);

    }

    /**
     * Now, terminate the slaves 
     */
    for (i = 1; i < mpi_size; i++){
        MPI_Send(map2d(npxc, module), 3, MPI_INT, i, terminate_tag, MPI_COMM_WORLD);
    }

    /**
     * FARM ENDS
     */
   
    /**
     * CLOSE HDF5 STORAGE
     */
    H5Dclose(dset_board);
    H5Dclose(dset_data);
    H5Sclose(mapspace);
    H5Sclose(memmapspace);
    H5Sclose(rawspace);
    H5Sclose(memrawspace);
    H5Gclose(data_group);
    H5Fclose(file_id);

    /**
     * Master can do something useful after the computations 
     */
   // printf("master out\n");
    query = lt_dlsym(module,"userdefined_masterOUT");
    query(mpi_size, &d, rawdata);

    return;
}

/**
 * SLAVE
 *
 */
static void slave(lt_dlhandle module, int mrl){

    int *tab=malloc(3*sizeof(*tab));
    int k = 0, i = 0, j = 0;
    //const char *dlr;

    module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR, qpx, qpc;
    
    masterData raw;
    masterData *rawdata;
    rawdata = malloc(sizeof(rawdata) + (mrl-1) * sizeof(MY_DATATYPE));
    
//    clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
   
    struct slaveData_t *s;
    s = makeSlaveData();
   
    /**
     * Slave can do something useful before computations
     */
    query = lt_dlsym(module, "userdefined_slaveIN");
    query(mpi_rank, &d, rawdata, s);

    qbeforeR = lt_dlsym(module, "userdefined_slave_beforeReceive");
    qbeforeR(mpi_rank, &d, rawdata, s);
    
    MPI_Recv(tab, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
    
    qafterR = lt_dlsym(module, "userdefined_slave_afterReceive");
    qafterR(mpi_rank, &d, rawdata, s);

    while(1){

     if(mpi_status.MPI_TAG == data_tag){ 
      
       /**
        * One pixel per each slave
        */
       if (method == 0){

          rawdata->coords[0] = tab[0];
          rawdata->coords[1] = tab[1];
          rawdata->coords[2] = tab[2];
          
          /**
           * DO SOMETHING HERE
           */
          qpx = lt_dlsym(module, "userdefined_pixelCompute");
          qpx(mpi_rank, mrl, &d, rawdata, s);
          
          qbeforeS = lt_dlsym(module, "userdefined_slave_beforeSend");
          qbeforeS(mpi_rank, &d, rawdata, s);
          
          position = 0;
          MPI_Pack(rawdata->coords, 3, MPI_INT, buffer, maxsize, &position, MPI_COMM_WORLD);
          MPI_Pack(rawdata->res, mrl, MY_MPI_DATATYPE, buffer, maxsize, &position, MPI_COMM_WORLD);
          MPI_Send(buffer, position, MPI_PACKED, mpi_dest, result_tag, MPI_COMM_WORLD);
          
          qafterS = lt_dlsym(module, "userdefined_slave_afterSend");
          qafterS(mpi_rank, &d, rawdata, s);
        
          qbeforeR = lt_dlsym(module, "userdefined_slave_beforeReceive");
          qbeforeR(mpi_rank, &d, rawdata, s);
          
          MPI_Recv(tab, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
          
          qafterR = lt_dlsym(module, "userdefined_slave_afterReceive");
          qafterR(mpi_rank, &d, rawdata, s);
        }

       if (method == 6){

          /**
           * Use userdefined pixelCoords method
           * and pixelCompute
           *
           */
          qpc = lt_dlsym(module, "userdefined_pixelCoords");
          qpc(mpi_rank, tab, &d, rawdata, s);
          
          qpx = lt_dlsym(module, "userdefined_pixelCompute");
          qpx(mpi_rank, mrl, &d, rawdata, s);
          
          qbeforeS = lt_dlsym(module, "userdefined_slave_beforeSend");
          qbeforeS(mpi_rank, &d, rawdata, s);
          
          position = 0;
          MPI_Pack(rawdata->coords, 3, MPI_INT, buffer, maxsize, &position, MPI_COMM_WORLD);
          MPI_Pack(rawdata->res, mrl, MY_MPI_DATATYPE, buffer, maxsize, &position, MPI_COMM_WORLD);
          MPI_Send(buffer, position, MPI_PACKED, mpi_dest, result_tag, MPI_COMM_WORLD);
          
          qafterS = lt_dlsym(module, "userdefined_slave_afterSend");
          qafterS(mpi_rank, &d, rawdata, s);
          
          qbeforeR = lt_dlsym(module, "userdefined_slave_beforeReceive");
          qbeforeR(mpi_rank, &d, rawdata, s);
          
          MPI_Recv(tab, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
          
          qafterR = lt_dlsym(module, "userdefined_slave_afterReceive");
          qafterR(mpi_rank, &d, rawdata, s);
       }
    }
      
     if(mpi_status.MPI_TAG == terminate_tag) break;     
    }
    
    /**
     * Slave can do something useful after computations
     */
    query = lt_dlsym(module, "userdefined_slaveOUT");
    query(mpi_rank, &d, rawdata, s);

    return;
}

/**
 * HELPER FUNCTION -- MAP 1D INDEX TO 2D ARRAY
 *
 */

int* map2d(int c, void *module){
   int *ind = malloc(3*sizeof(*ind));
   int x, y;
   x = d.xres;
   y = d.yres;
   module_query_void_f qpcm;
  
   ind[2] = c; //we need number of current pixel to store too

   /**
    * Method 0: one pixel per each slave
    */
   if(method == 0){
    if(c < y) ind[0] = c / y; ind[1] = c;
    if(c > y - 1) ind[0] = c / y; ind[1] = c % y;
   }

   /**
    * Method 6: user defined control
    */
   if(method == 6){
    qpcm = lt_dlsym(module, "userdefined_pixelCoordsMap"); 
    qpcm(ind, c, x, y);
   }

   return ind;
}

/**
 * Clears arrays
 */
void clearArray(MY_DATATYPE* array, int no_of_items_in_array){

	int i;
	for(i = 0;i < no_of_items_in_array; i++){
		array[i] = 0.0;
	}

	return;
}

static void master_test(lt_dlhandle mod){
  module_query_void_f qq;
  qq = lt_dlsym(mod,"mpifarm_module_query");
  return;
}
static void slave_test(lt_dlhandle mod){
  module_query_void_f qq;
    
  struct slaveData_t *s;
  s = makeSlaveData();
   
  qq = lt_dlsym(mod,"mpifarm_module_query");
  return;
}
