/* MPI FARM SKELETON
 * Task farm model
 *
 * by mariusz slonina <mariusz.slonina@gmail.com>
 * with a little help of kacper kowalik <xarthisius.kk@gmail.com>
 *
 * BE CAREFUL -- almost no error handling (TODO)
 */
#include "mpifarm.h"

int* map2d(int, void*, configData *d);
void master(void*, configData *d);
void slave(void*, configData *d);
void clearArray(MY_DATATYPE*,int);
void buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr);
void buildDefaultConfigType(configData *d, MPI_Datatype* defaultConfigType_ptr);
int readDefaultConfig(char* inifile, char* sep, char* comm, configData *cd);

/* settings */ 
int allopts = 0; //number of options read

/* mpi */ 
MPI_Status mpi_status;
int mpi_rank, mpi_size, mpi_dest = 0, mpi_tag = 0;
int source_tag = 0, data_tag = 2, result_tag = 59, terminate_tag = 99; //data tags

int masteralone = 0;

int i = 0, j = 0, k = 0, opts = 0, n = 0;
    
struct slaveData_t *s;

 
/**
 * MAIN
 *
 */
int main(int argc, char *argv[]){  

  char *module_name;
  char module_file[256];
  void* module;
  char *dlresult;
  char optvalue;

  void *prestartmode;
  int restartmode = 0;
  int poptflags = 0;
  int error;

  configData cd;
  configData *d;

  /*  Set defaults */
  inifile = CONFIG_FILE_DEFAULT;
  module_name = MODULE_DEFAULT;

  strcpy(cd.name,NAME_DEFAULT);
  strcpy(cd.datafile, MASTER_FILE_DEFAULT);
  strcpy(cd.module, MODULE_DEFAULT);
  cd.xres = 5;
  cd.yres = 5;
  cd.method = 0;
  cd.mrl = 13;
  cd.dump = 2000;
 
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  
  /**
   * BY DESIGN
   * Master cannot work alone.
   */
  if (mpi_size == 1){
     printf("You should have at least one slave!\n");
     masteralone = 1;

     MPI_Abort(MPI_COMM_WORLD, 911);
  }

  /**
   * Read config values
   */
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

  poptContext poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, POPT_CONTEXT_POSIXMEHARDER);

  if(mpi_rank == 0){
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
  }

  if (optvalue < -1){
    fprintf(stderr, "%s: %s\n",
        poptBadOption(poptcon, POPT_BADOPTION_NOALIAS), 
        poptStrerror(optvalue));
        MPI_Finalize();
        return 1;
  }
  
  
  /**
   * Inform slaves what is all about
   */
  //printf("configData\n");
  MPI_Datatype defaultConfigType;

  //printf("read config\n");
  if(mpi_rank == 0){
   allopts = readDefaultConfig(inifile, sep, comm, &cd);
    if (cd.xres == 0 || cd.yres == 0){
       printf("X/Y map resolution should not be set to 0!\n");
       printf("If You want to do only one simulation, please set xres = 1, yres = 1\n");
       MPI_Abort(MPI_COMM_WORLD, 911);
    }
    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, 0, MPI_COMM_WORLD);
  }else{
    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, 0, MPI_COMM_WORLD);
  }
 
  
  /**
   * Module loading, if option -m is not set, use default
   */
  sprintf(module_file, "mpifarm_module_%s.so", module_name);
  
  module = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!module){
    printf("Cannot load module '%s': %s\n", module_name, dlerror()); 
    MPI_Abort(MPI_COMM_WORLD, 913);
  }

  init = dlsym(module, "mpifarm_module_init");

  /* test only, do not use it! */
  //struct yourdata *pointer;
  //pointer = makeyourdata();
  
  init();

  query = dlsym(module, "mpifarm_module_query");
  query();

  if(mpi_rank == 0) {
      master(module, &cd);
	} else {
      slave(module, &cd); 
  }

  cleanup = dlsym(module, "mpifarm_module_cleanup");

  printf("CLEANUP\n");
  cleanup();
  printf("DLCLOSE\n");
  dlclose(module);
  printf("module closed\n");
  printf("FINALIZE\n");
  MPI_Type_free(&defaultConfigType);
	MPI_Finalize();

  return 0;
}

/**
 * MASTER
 *
 */
void master(void* module, configData *d){

   int *tab; 
	 MY_DATATYPE rdata[d->mrl][1];
   int i = 0, k = 0, j = 0;
   int nodes = 0;
   int farm_res = 0;
   int npxc = 0; //number of all sent pixels
   int count = 0; //number of pixels to receive
   
   module_query_void_f queryIN, queryOUT, qbeforeS, qafterS, qbeforeR, qafterR;
   module_query_int_f qr;
  
   masterData raw;
   masterData *rawdata;
 
   /* hdf */
   hid_t file_id, dset_board, dset_data, dset_config, data_group;
   hid_t configspace, configmemspace, mapspace, memmapspace, rawspace, memrawspace, maprawspace; 
   hid_t plist_id;
   hsize_t dimsf[2], dimsr[2], co[2], rco[2], off[2], stride[2];
   int fdata[1][1];
   herr_t hdf_status;
   hid_t hdf_config_datatype;


   /**
    * Allocate memory for rawdata.res array
    */
   rawdata = malloc(sizeof(rawdata) + (d->mrl-1) * sizeof(MY_DATATYPE));

   clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
   
   printf("MAP RESOLUTION = %dx%d.\n", d->xres, d->yres);
   printf("MPI_SIZE = %d.\n", mpi_size);
   printf("METHOD = %d.\n", d->method);

   /**
    * Master can do something useful before computations
    */
   query = dlsym(module, "userdefined_masterIN");
   query(mpi_size, d);

   /**
    * Align farm resolution for given method
    */

   if (d->method == 0) farm_res = d->xres*d->yres; //one pixel per each slave
   if (d->method == 1) farm_res = d->xres; //sliceX
   if (d->method == 2) farm_res = d->yres; //sliceY
   if (d->method == 6){
     //qr = dlsym(module, "userdefined_farmResolution");
     //farm_res = qr(d->xres, d->yres);
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
    file_id = H5Fcreate(d->datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    
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
    dimsf[0] = d->xres;
    dimsf[1] = d->yres;
    mapspace = H5Screate_simple(HDF_RANK, dimsf, NULL);
    
    dset_board = H5Dcreate(file_id, DATABOARD, H5T_NATIVE_INT, mapspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* Master data group */
    data_group = H5Gcreate(file_id, DATAGROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
    /* Result data space  */
    dimsr[0] = d->xres*d->yres;
    dimsr[1] = d->mrl;
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
    rco[1] = d->mrl;
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
    * Send tasks to all slaves,
    * remembering what the farm resolution is
    */
   //qbeforeS = dlsym(module,"userdefined_master_beforeSend");
   //qafterS = dlsym(module, "userdefined_master_afterSend");
   for (i = 1; i < nodes; i++){
     //qbeforeS(i,d,rawdata);

     MPI_Send(map2d(npxc, module, d), 3, MPI_INT, i, data_tag, MPI_COMM_WORLD);
     
     //qafterS(i,d,rawdata);
     
     count++;
     npxc++;
   }
   
   /**
    * We don't want to have slaves idle, so, if there are some idle slaves
    * terminate them
    */
   for(i = nodes; i < mpi_size; i++){
    // MPI_Send(map2d(npxc, module), 3, MPI_INT, i, terminate_tag, MPI_COMM_WORLD);
   }

   /**
    * Receive data and send tasks
    */
   MPI_Datatype masterResultsType;
   buildMasterResultsType(d->mrl, rawdata, &masterResultsType);
   
   while (1) {
    
     //qbeforeR = dlsym(module, "userdefined_master_beforeReceive");
     //qbeforeR(d,rawdata);
      
     MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
     count--;
      
      //qafterR = dlsym(module, "userdefined_master_afterReceive");
      //qafterR(mpi_status.MPI_SOURCE, d, rawdata);
      
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
      for (j = 0; j < d->mrl; j++){
        rdata[j][0] = rawdata->res[j];
      }
      
      H5Sselect_hyperslab(rawspace, H5S_SELECT_SET, off, NULL, rco, NULL);
      hdf_status = H5Dwrite(dset_data, H5T_NATIVE_DOUBLE, memrawspace, rawspace, H5P_DEFAULT, rdata);

      /**
       * Send next task to the slave
       */
      if (npxc < farm_res){
       
        //qbeforeS = dlsym(module, "userdefined_master_beforeSend");
        //qbeforeS(mpi_status.MPI_SOURCE, &d, rawdata);
         
        MPI_Send(map2d(npxc, module, d), 3, MPI_INT, mpi_status.MPI_SOURCE, data_tag, MPI_COMM_WORLD);
        npxc++;
        count++;
        
        //qafterS = dlsym(module, "userdefined_master_afterSend");
        //qafterS(mpi_status.MPI_SOURCE, d, rawdata);
      } else {
        break;
      }
    }
   //MPI_Type_free(&masterResultsType);

    /** 
     * No more work to do, receive the outstanding results from the slaves 
     * We've got exactly 'count' messages to receive 
     */
    for (i = 0; i < count; i++){
      
      //qbeforeR = dlsym(module, "userdefined_master_beforeReceive");
      //qbeforeR(d, rawdata);
        
      MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      
      //qafterR = dlsym(module, "userdefined_master_afterReceive");
      //qafterR(mpi_status.MPI_SOURCE, d, rawdata);
        
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
        
        for (j = 0; j < d->mrl; j++){
          rdata[j][0] = rawdata->res[j];
        }
      
        H5Sselect_hyperslab(rawspace, H5S_SELECT_SET, off, NULL, rco, NULL);
        hdf_status = H5Dwrite(dset_data, H5T_NATIVE_DOUBLE, memrawspace, rawspace, H5P_DEFAULT, rdata);

    }
   MPI_Type_free(&masterResultsType);

    /**
     * Now, terminate the slaves 
     */
    for (i = 1; i < nodes; i++){
        MPI_Send(map2d(npxc, module, d), 3, MPI_INT, i, terminate_tag, MPI_COMM_WORLD);
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
    //query = dlsym(module,"userdefined_masterOUT");
    //query(nodes, d, rawdata);

    //free(optstohdf);
    //free(rawdata);
    return;
}

/**
 * SLAVE
 *
 */
void slave(void* module, configData *d){

    int *tab=malloc(3*sizeof(*tab));
    int k = 0, i = 0, j = 0;
    MPI_Datatype masterResultsType;

    module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR, qpx, qpc;
    
    masterData raw;
    masterData *rawdata;
    rawdata = malloc(sizeof(rawdata) + (d->mrl-1) * sizeof(MY_DATATYPE));
    
    clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
   
    /**
     * Slave can do something useful before computations
     */
  //  query = dlsym(module, "userdefined_slaveIN");
  //  query(mpi_rank, d, rawdata);

    //qbeforeR = dlsym(module, "userdefined_slave_beforeReceive");
    //qbeforeR(mpi_rank, d, rawdata, s);
    
    MPI_Recv(tab, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
    
    //qafterR = dlsym(module, "userdefined_slave_afterReceive");
    //qafterR(mpi_rank, d, rawdata, s);

    buildMasterResultsType(d->mrl, rawdata, &masterResultsType);
    
    while(1){

     if(mpi_status.MPI_TAG == terminate_tag) break;     
     if(mpi_status.MPI_TAG == data_tag){ 
      
       /**
        * One pixel per each slave
        */
       if (d->method == 0){

          rawdata->coords[0] = tab[0];
          rawdata->coords[1] = tab[1];
          rawdata->coords[2] = tab[2];
       }
       if (d->method == 6){

          /**
           * Use userdefined pixelCoords method
           */
      //    qpc = dlsym(module, "userdefined_pixelCoords");
      //    qpc(mpi_rank, tab, d, rawdata, s);
       } 
          /**
           * DO SOMETHING HERE
           */
        //  qpx = dlsym(module, "userdefined_pixelCompute");
        //  qpx(mpi_rank, d, rawdata, s);
          
        //  qbeforeS = dlsym(module, "userdefined_slave_beforeSend");
        //  qbeforeS(mpi_rank, d, rawdata, s);
         
          MPI_Send(rawdata, 1, masterResultsType, mpi_dest, result_tag, MPI_COMM_WORLD);

        //  qafterS = dlsym(module, "userdefined_slave_afterSend");
        //  qafterS(mpi_rank, d, rawdata, s);
        
        //  qbeforeR = dlsym(module, "userdefined_slave_beforeReceive");
        //  qbeforeR(mpi_rank, d, rawdata, s);
          
          MPI_Recv(tab, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
          
        //  qafterR = dlsym(module, "userdefined_slave_afterReceive");
        //  qafterR(mpi_rank, d, rawdata, s);
        }
      
    }
    
    /**
     * Slave can do something useful after computations
     */
    //query = dlsym(module, "userdefined_slaveOUT");
    //query(mpi_rank, d, rawdata, s);

//    free(rawdata);
    return;
}

/**
 * HELPER FUNCTION -- MAP 1D INDEX TO 2D ARRAY
 *
 */

int* map2d(int c, void* module, configData *d){
   int *ind = malloc(3*sizeof(*ind));
   int x, y;
   x = d->xres;
   y = d->yres;
   module_query_void_f qpcm;
  
   ind[2] = c; //we need number of current pixel to store too

   /**
    * Method 0: one pixel per each slave
    */
   if(d->method == 0){
    if(c < y) ind[0] = c / y; ind[1] = c;
    if(c > y - 1) ind[0] = c / y; ind[1] = c % y;
   }

   /**
    * Method 6: user defined control
    */
   if(d->method == 6){
    //qpcm = dlsym(module, "userdefined_pixelCoordsMap"); 
    //qpcm(ind, c, x, y);
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

/**
 * DEFAULT CONFIG FILE PARSER
 */
int readDefaultConfig(char* inifile, char* sep, char* comm, configData *d){

  int i = 0, k = 0, opts = 0, offset = 0;

  //printf("start parse...");
  opts = parseConfigFile(inifile, sep, comm);
  //printf(" parsed %d opts\n",opts);
  //printAll(opts);
	for(i = 0; i < opts; i++){
  //  printf("loop %d\n", i);
    if(strcmp(configSpace[i].space,"default") == 0){
		  for(k = 0; k < configSpace[i].num; k++){
			  if(strcmp(configSpace[i].options[k].name,"name") == 0){
          strcpy(d->name,configSpace[i].options[k].value);
          strcpy(d->datafile, strcat(configSpace[i].options[k].value,"-master.h5"));  
        }
			  if(strcmp(configSpace[i].options[k].name,"module") == 0){
          strcpy(d->module,configSpace[i].options[k].value);
        }
			  if(strcmp(configSpace[i].options[k].name,"xres") == 0) d->xres = atoi(configSpace[i].options[k].value);  
        if(strcmp(configSpace[i].options[k].name,"yres") == 0) d->yres = atoi(configSpace[i].options[k].value); 
			  if(strcmp(configSpace[i].options[k].name,"method") == 0) d->method = atoi(configSpace[i].options[k].value); 
			  if(strcmp(configSpace[i].options[k].name,"mrl") == 0) d->mrl = atoi(configSpace[i].options[k].value); 
    }
    }
    if(strcmp(configSpace[i].space,"logs") == 0){
		  for(k = 0; k < configSpace[i].num; k++){
			  if(strcmp(configSpace[i].options[k].name,"dump") == 0) d->dump = atoi(configSpace[i].options[k].value); 
      }
    }
	}
  //printf("after loop\n");

  return opts;
}

/**
 * MPI DERIVED DATATYPE 
 * for master result Send/Recv
 */
void buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr){

  int block_lengths[2];
  MPI_Aint displacements[2];
  MPI_Datatype typelist[2];
  MPI_Aint addresses[3];

  typelist[0] = MPI_INT;
  typelist[1] = MPI_DOUBLE;

  block_lengths[0] = 3;
  block_lengths[1] = mrl;

  MPI_Address(md, &addresses[0]);
  MPI_Address(md->coords, &addresses[1]);
  MPI_Address(md->res, &addresses[2]);

  displacements[0] = addresses[1] - addresses[0];
  displacements[1] = addresses[2] - addresses[0];

  MPI_Type_struct(2, block_lengths, displacements, typelist, masterResultsType_ptr);
  MPI_Type_commit(masterResultsType_ptr);
}

/**
 * MPI DERIVED TYPE
 * for reading default config file
 */
void buildDefaultConfigType(configData *d, MPI_Datatype* defaultConfigType_ptr){
  
  int block_lengths[8];
  MPI_Aint displacements[8];
  MPI_Datatype typelist[8];
  MPI_Aint addresses[9];

  typelist[0] = MPI_CHAR; //xres
  typelist[1] = MPI_CHAR; //yres
  typelist[2] = MPI_CHAR; //method
  typelist[3] = MPI_INT; //dump
  typelist[4] = MPI_INT; //master result length
  typelist[5] = MPI_INT; //problem name
  typelist[6] = MPI_INT; //master datafile
  typelist[7] = MPI_INT; //module

  block_lengths[0] = 256;
  block_lengths[1] = 260;
  block_lengths[2] = 256;
  block_lengths[3] = 1;
  block_lengths[4] = 1;
  block_lengths[5] = 1;
  block_lengths[6] = 1;
  block_lengths[7] = 1;

  MPI_Address(d, &addresses[0]);
  MPI_Address(&(d->name), &addresses[1]);
  MPI_Address(&(d->datafile), &addresses[2]);
  MPI_Address(&(d->module), &addresses[3]);
  MPI_Address(&(d->xres), &addresses[4]);
  MPI_Address(&(d->yres), &addresses[5]);
  MPI_Address(&(d->method), &addresses[6]);
  MPI_Address(&(d->mrl), &addresses[7]);
  MPI_Address(&(d->dump), &addresses[8]);

    displacements[0] = addresses[1] - addresses[0];
    displacements[1] = addresses[2] - addresses[0];
    displacements[2] = addresses[3] - addresses[0];
    displacements[3] = addresses[4] - addresses[0];
    displacements[4] = addresses[5] - addresses[0];
    displacements[5] = addresses[6] - addresses[0];
    displacements[6] = addresses[7] - addresses[0];
    displacements[7] = addresses[8] - addresses[0];

  MPI_Type_struct(8, block_lengths, displacements, typelist, defaultConfigType_ptr);
  MPI_Type_commit(defaultConfigType_ptr);

}

