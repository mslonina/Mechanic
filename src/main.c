/* MPI FARM SKELETON
 * Task farm model
 *
 * by mariusz slonina <mariusz.slonina@gmail.com>
 * with a little help of kacper kowalik <xarthisius.kk@gmail.com>
 *
 * FIX ME! -- only partial error handling:
 * 1) HDF5 error handling with H5E (including file storage)
 * 2) dlopen error handling (done)
 * 3) user input error handling (done in libreadconfig)
 * 4) malloc error handling
 * 
 * Maybe void functions should return int value?
 *
 */
#include "mechanic.h"
#include "mechanic-internals.h"

/**
 * MAIN
 *
 */
int main(int argc, char *argv[]){  

  int allopts = 0; //number of options read
  int i = 0, j = 0, k = 0, opts = 0, n = 0;
  
  char* module_name;
  char* name;
  char* restartname;
  char module_file[MECHANIC_FILE];
  char* dlresult;
  char optvalue;
  void* handler;
  char oldfile[MECHANIC_FILE_OLD];
  char restartfile[MECHANIC_FILE];

  int poptflags = 0;
  int error;
  int rc;
  int configfile = 0;
  int restartmode = 0;

  struct stat st; //stat.h
  
  module_query_int_f qd;

  hid_t file_id;

  configData cd; //struct for command line args
  moduleInfo md;

  MPI_Datatype defaultConfigType;
  
  /* Assign default config values */
  name = MECHANIC_NAME_DEFAULT;
  restartname = MECHANIC_NAME_DEFAULT;
  inifile = MECHANIC_CONFIG_FILE_DEFAULT;
  module_name = MECHANIC_MODULE_DEFAULT;
  
  sprintf(cd.name, "%s", MECHANIC_NAME_DEFAULT);
  sprintf(cd.datafile, "%s", MECHANIC_MASTER_FILE_DEFAULT); 
  sprintf(cd.module, "%s", MECHANIC_MODULE_DEFAULT);
  cd.xres = MECHANIC_XRES_DEFAULT;
  cd.yres = MECHANIC_YRES_DEFAULT;
  cd.method = MECHANIC_METHOD_DEFAULT;
  cd.mrl = MECHANIC_MRL_DEFAULT;
  cd.checkpoint = MECHANIC_DUMP_DEFAULT;
  cd.restartmode = 0;

  LRC_configNamespace cs[] = {
    {"default",{{"name", "", LRC_CHAR},{"xres", "", LRC_INT},{"yres", "", LRC_INT},
        {"method", "", LRC_INT},{"mrl", "", LRC_INT},{"module", "", LRC_CHAR},},6},
    {"logs", {{"checkpoint", "", LRC_INT}}, 1}
  };
  allopts = 2;

  /* This is the easiest way to assign values ic cs[] */
  sprintf(cs[0].options[0].value,"%s",MECHANIC_NAME_DEFAULT);
  sprintf(cs[0].options[1].value,"%d",MECHANIC_XRES_DEFAULT);
  sprintf(cs[0].options[2].value,"%d",MECHANIC_YRES_DEFAULT);
  sprintf(cs[0].options[3].value,"%d",MECHANIC_METHOD_DEFAULT);
  sprintf(cs[0].options[4].value,"%d",MECHANIC_MRL_DEFAULT);
  sprintf(cs[0].options[5].value,"%s",MECHANIC_MODULE_DEFAULT);
  sprintf(cs[1].options[0].value,"%d",MECHANIC_DUMP_DEFAULT);

  /* Assign allowed values */
  LRC_configTypes ct[] = {
    {"default", "name", LRC_CHAR},
    {"default", "xres", LRC_INT},
    {"default", "yres", LRC_INT},
    {"default", "method", LRC_INT},
    {"default", "mrl", LRC_INT},
    {"default", "module", LRC_CHAR},
    {"logs", "checkpoint", LRC_INT}
  };

  /* Number of allowed values */
  int numCT = 7;

  /**
   * POPT CONFIG
   */
  struct poptOption mechanic_poptHelpOptions[] = {
    { NULL, '\0', POPT_ARG_CALLBACK, (void *)mechanic_displayUsage, 0, NULL, NULL },
    { "help", '?', 0, NULL, (int)'?', "Show this help message", NULL },
    { "usage", '\0', 0, NULL, (int)'u', "Display brief usage message", NULL },
      POPT_TABLEEND
    };
  
  struct poptOption cmdopts[] = {
    MECHANIC_POPT_AUTOHELP
    {"name", 'n', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &name, 0,
    "problem name", "NAME"},
    {"config", 'c', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &inifile, 0,
    "config file", "CONFIG"},
    {"module", 'p', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &module_name, 0,
    "module", "MODULE"},
    {"method", 'm', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.method, 0,
    "method", "METHOD"},
    {"xres", 'x', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.xres, 0,
    "x resolution", "XRES"},
    {"yres", 'y', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.yres, 0,
    "y resolution", "YRES"},
    {"mrl", 'l', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.mrl, 0,
    "master result array length", "LENGTH"},
    {"checkpoint", 'd', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.checkpoint, 0,
    "how often write checkpoint file", "CHECKPOINT"},
    {"restart", 'r', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.restartmode, 1,
    "restart mode [TODO]","RESTART"},
    POPT_TABLEEND
  };

  const char** rv;
  struct poptAlias restartAlias = {
    "restart", 'r', 3, rv
  };
 
  /* MPI INIT */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  
  /* HDF5 INIT */
  H5open();
  
  /**
   * FIX ME! 
   * 1) move farm to core module
   * 2) allow user to change modes:
   *  -- farm mode (default)
   *  -- grid mode (multifarm)
   *  -- grid with communication between slaves
   *  -- masteralone 
   */
  if (mpi_size == 1){
     printf("You should have at least one slave!\n");
     MPI_Finalize();
     return 0;
  }

  /* Parse command line */
  poptContext poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);
  poptAddAlias(poptcon, restartAlias, 0);

  /**
   * 1) Read defaults
   * 2) Read args
   * 3) If one of arg is -c|--config read config file 
   * 4) Override config values according to args
   *
   * If default config file is not present, use fixed defaults.
   * If option -c is set, but the file doesn't exit, abort.
   *
   */
  optvalue = poptGetNextOpt(poptcon);
    
  /* Bad option */
  if (optvalue < -1){
    if(mpi_rank == 0){
      fprintf(stdout, "%s: %s\n", poptBadOption(poptcon, POPT_BADOPTION_NOALIAS), poptStrerror(optvalue));
      poptPrintHelp(poptcon, stdout, poptflags);
     }
     poptFreeContext(poptcon);
     MPI_Finalize();
     return 0;
  }
  
  if(mpi_rank == 0) welcome();
  
  /* Long help message set */
  if (help == 1){
    if(mpi_rank == 0) poptPrintHelp(poptcon, stdout, 0);
    poptFreeContext(poptcon);
    MPI_Finalize();
    return 0;
  }
   
  /* Brief help message set */
  if (usage == 1){
    if(mpi_rank == 0) poptPrintUsage(poptcon, stdout, 0);
    poptFreeContext(poptcon);
    MPI_Finalize();
    return 0;
  }
  
  /**
   * RESTART MODE
   *
   * TODO:
   * 1) -r should take problem name as an argument -- DONE
   * 2) check if master file exists
   *  -- if exists, check if it's not corrupted (is hdf5) -- DONE
   *  -- if it's corrupted, check for old copy
   *  -- if the copy exists, check if it's not corrupted
   *  -- if the copy is corrupted or does not exist, abort restart
   * 3) read config values from master file, ignore all command line opts etc.
   * 4) check board status
   *  -- if board is full, abort, because we don't need restart completed simulation
   *  -- if board is not full, check for empty pixels and compute them
   *      (map2d function should check if pixel is computed or not)
   * 5) perform normal operations, i.e. write new restart files
   */

  if (cd.restartmode == 1){

    if(mpi_rank == 0){
      printf("We are in restart mode... Checking files... ");

      sprintf(restartfile, "%s-master.h5", restartname);
    
      if(stat(restartfile, &st) == 0){
      
        if(H5Fis_hdf5(restartfile) > 0) 
          printf("We can restart the simulation\n");
        else
          printf("Cannot restart the simulation\n");

      }else{
        printf("Master file does not exist\n");
      }

    }
     MPI_Finalize();
     return 0;
  }

  /* Config file set */
  if(strcmp(inifile, MECHANIC_CONFIG_FILE_DEFAULT) != 0 && cd.restartmode == 0){
    configfile = 1;
  }


  /* Reset options, we need to reuse them
   * to override values read from specified config file */
  if(mpi_rank == 0){

    /* Read config file */
    allopts = readDefaultConfig(inifile, cs, ct, numCT, configfile);

    /**
     * Config file parsed. All read values becomes
     * new defaults 
     */
    assignConfigValues(allopts, &cd, cs, configfile, 0);

    /* Reset pointers */
    name = cd.name;
    module_name = cd.module;
  
    /* Override the values by command line args */
    poptResetContext(poptcon);
    poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);
    optvalue = poptGetNextOpt(poptcon);
  
    /* In case of any command line arg, 
     * we need to reassign config values. */
    if(strcmp(name, cd.name) != 0) sprintf(cd.name,"%s",name);
    if(strcmp(module_name, cd.module) != 0) sprintf(cd.module,"%s",module_name);
    assignConfigValues(allopts, &cd, cs, configfile, 1);
   
    /* Security check */
    if (cd.xres == 0 || cd.yres == 0){
       printf("X/Y map resolution should not be set to 0!\n");
       printf("If You want to do only one simulation, please set xres = 1, yres = 1\n");
       MPI_Abort(MPI_COMM_WORLD, MECHANIC_ERR_MPI);
    }

    printf("\n-> Mpifarm will use these startup values:\n\n");
    LRC_printAll(allopts,cs);
  }

  poptFreeContext(poptcon);
  
  
  /* Config file read */
  if(mpi_rank == 0){
    /* Need support for restart mode here */
    if(stat(cd.datafile,&st) == 0){
      sprintf(oldfile,"old-%s",cd.datafile);
      printf("-> File %s exists!\n", cd.datafile);
      printf("-> I will back it up for You now\n");
      printf("-> Backuped file: %s\n",oldfile);
      rename(cd.datafile,oldfile);
    }

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
    * In such case, please edit slaveIN/OUT functions in your module.
    *
    * FIX ME!:
    * Add logs functionality
    *
    */

  /* Create master datafile */
  file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  H5createMasterDataScheme(file_id, &cd);

  /* First of all, save configuration */
  LRC_writeHdfConfig(file_id, cs, allopts);
  H5Fclose(file_id);

  /**
   * MPI CONFIG BCAST
   * Inform slaves what it is all about.
   */

    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, MECHANIC_MPI_DEST, MPI_COMM_WORLD);
  
  }else{
    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, MECHANIC_MPI_DEST, MPI_COMM_WORLD);
  }
  
  /**
   * MODULE LOAD
   * If option -p is not set, use default.
   */
  sprintf(module_file, "mechanic_module_%s.so", module_name);
 
  md.name = module_name;
  handler = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!handler){
    printf("Cannot load module '%s': %s\n", module_name, dlerror()); 
    MPI_Abort(MPI_COMM_WORLD, MECHANIC_ERR_MODULE);
  }

  init = load_sym(handler,&md, "init", MECHANIC_MODULE_ERROR);
  if(init) init(&md);

  query = load_sym(handler,&md, "query", MECHANIC_MODULE_SILENT);
  if(query) query();

  /**
   * NOW, DO IT!
   */
  if(mpi_rank == 0) {
      master(handler, &md, &cd, restartmode);
	} else {
      slave(handler, &md, &cd); 
  }

  cleanup = load_sym(handler, &md, "cleanup", MECHANIC_MODULE_ERROR);
  if(cleanup) cleanup();

  dlclose(handler);
  MPI_Type_free(&defaultConfigType);
  
  /* HDF5 FINALIZE */
  H5close();
	
  /* MPI FINALIZE */
  MPI_Finalize();

  return 0;
}

void welcome(){

  printf("MPIFARM v.1.0 %s\n\tauthor: %s\n\tbugs: %s\n\n",
          MECHANIC_VERSION, MECHANIC_AUTHOR, MECHANIC_EMAIL);

}
