/* MPI FARM SKELETON
 * Task farm model
 *
 * by mariusz slonina <mariusz.slonina@gmail.com>
 * with a little help of kacper kowalik <xarthisius.kk@gmail.com>
 *
 * BE CAREFUL -- almost no error handling (TODO)
 */
#include "mpifarm.h"

/* Global settings */ 
int allopts = 0; //number of options read

int masteralone = 0;

int i = 0, j = 0, k = 0, opts = 0, n = 0;
    
/**
 * MAIN
 *
 */
int main(int argc, char *argv[]){  

  char* module_name;
  char module_file[256];
  void* module;
  char* dlresult;
  char optvalue;

  void* prestartmode;
  int restartmode = 0;
  int poptflags = 0;
  int error;
  int configfile = 0;
  int x = 0, y = 0, l = 0, method = 0;

  configData cd;
  configData* d;

  /* Set defaults */
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
 
  /**
   * POPT CONFIG
   */
  struct poptOption cmdopts[] = {
    POPT_AUTOHELP
    {"name", 'n', POPT_ARG_STRING || POPT_ARGFLAG_SHOW_DEFAULT, &cd.name, 'n',
    "problem name", "NAME"},
    {"config", 'c', POPT_ARG_STRING || POPT_ARGFLAG_SHOW_DEFAULT, &inifile, 'c',
    "config file", "CONFIG"},
    {"module", 'p', POPT_ARG_STRING || POPT_ARGFLAG_SHOW_DEFAULT, &module_name, 'p',
    "module", "MODULE"},
    {"method", 'm', POPT_ARG_INT || POPT_ARGFLAG_SHOW_DEFAULT, &cd.method, 'm',
    "method", "METHOD"},
    {"xres", 'x', POPT_ARG_INT || POPT_ARGFLAG_SHOW_DEFAULT, &cd.xres, 'x',
    "x resolution", "XRES"},
    {"yres", 'y', POPT_ARG_INT || POPT_ARGFLAG_SHOW_DEFAULT, &cd.yres, 'y',
    "y resolution", "YRES"},
    {"mrl", 'l', POPT_ARG_INT || POPT_ARGFLAG_SHOW_DEFAULT, &cd.mrl, 'l',
    "master result array length", "LENGTH"},
    {"restart", 'r', POPT_ARG_NONE, &prestartmode, 'r',
    "restart mode [TODO]","RESTART"},
    POPT_TABLEEND
  };
 
  /* MPI INIT */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  
  /* HDF5 INIT */
  H5open();
 
  /* Parse command line */
  poptContext poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, POPT_CONTEXT_POSIXMEHARDER);

  /**
   * 1) read defaults
   * 2) read args
   * 3) if one of arg is -c|--config read config file 
   * 4) override config values according to args
   */
  while(((optvalue = poptGetNextOpt(poptcon)) >= 0) && configfile != 1){
    switch (optvalue){
      case 'c':
        configfile == 1;
        break;
      case 'm':
  //      method = poptGetOptArg(poptcon);
        break;
      case 'x':
    //    x = poptGetOptArg(poptcon);
        break;
      case 'y':
      //  y = poptGetOptArg(poptcon);
        break;
      case 'l':
      //  l = poptGetOptArg(poptcon);
        break;
      case '?':
        poptPrintHelp(poptcon, stderr, poptflags);
      case 'r':
        printf("TODO: restart mode\n");
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
   * BY DESIGN
   * Master cannot work alone.
   */
  if (mpi_size == 1){
     printf("You should have at least one slave!\n");
     masteralone = 1;

     MPI_Abort(MPI_COMM_WORLD, 911);
  }
  
  /**
   * MPI CONFIG BCAST
   * Inform slaves what is all about.
   */
  MPI_Datatype defaultConfigType;

  if(mpi_rank == 0){
      allopts = readDefaultConfig(inifile, &cd);
    if (cd.xres == 0 || cd.yres == 0){
       printf("X/Y map resolution should not be set to 0!\n");
       printf("If You want to do only one simulation, please set xres = 1, yres = 1\n");
       MPI_Abort(MPI_COMM_WORLD, 911);
    }
    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, MPI_DEST, MPI_COMM_WORLD);
  }else{
    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, MPI_DEST, MPI_COMM_WORLD);
  }
  
  /**
   * MODULE LOAD
   * If option -m is not set, use default.
   */
  sprintf(module_file, "mpifarm_module_%s.so", module_name);
  
  module = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!module){
    printf("Cannot load module '%s': %s\n", module_name, dlerror()); 
    MPI_Abort(MPI_COMM_WORLD, 913);
  }

  init = load_sym(module, "mpifarm_module_init", MODULE_ERROR);
  if(init) init();

  query = load_sym(module, "mpifarm_module_query", MODULE_SILENT);
  if(query) query();

  /**
   * NOW, DO IT!
   */
  if(mpi_rank == 0) {
      master(module, &cd);
	} else {
      slave(module, &cd); 
  }

  cleanup = load_sym(module, "mpifarm_module_cleanup", MODULE_ERROR);
  if(cleanup) cleanup();

  dlclose(module);
  poptFreeContext(poptcon);
  MPI_Type_free(&defaultConfigType);
  
  /* HDF5 FINALIZE */
  H5close();
	
  /* MPI FINALIZE */
  MPI_Finalize();

  return 0;
}


