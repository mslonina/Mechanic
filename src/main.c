/* MPI FARM SKELETON
 * Task farm model
 *
 * by mariusz slonina <mariusz.slonina@gmail.com>
 * with a little help of kacper kowalik <xarthisius.kk@gmail.com>
 *
 * BE CAREFUL -- only partial error handling (TODO)
 */
#include "mpifarm.h"

/* Global settings */ 
int allopts = 0; //number of options read

int i = 0, j = 0, k = 0, opts = 0, n = 0;
    
/**
 * MAIN
 *
 */
int main(int argc, char *argv[]){  

  char* module_name;
  char* name;
  char module_file[MAX_VALUE_LENGTH];
  char* dlresult;
  char optvalue;
  void* handler;
  module_query_int_f qd;
  char oldfile[MAX_VALUE_LENGTH];

  int poptflags = 0;
  int error;
  int configfile = 0;

  struct stat st; //stat.h

  hid_t file_id;

  configData cd; //struct for command line args
  moduleInfo md;

  MPI_Datatype defaultConfigType;
  
  /* Assign default config values */
  name = NAME_DEFAULT;
  inifile = CONFIG_FILE_DEFAULT;
  module_name = MODULE_DEFAULT;
  
  sprintf(cd.name,"%s",NAME_DEFAULT);
  sprintf(cd.datafile,"%s", MASTER_FILE_DEFAULT); 
  sprintf(cd.module,"%s",MODULE_DEFAULT);
  cd.xres = XRES_DEFAULT;
  cd.yres = YRES_DEFAULT;
  cd.method = METHOD_DEFAULT;
  cd.mrl = MRL_DEFAULT;
  cd.dump = DUMP_DEFAULT;
  cd.restartmode = 0;

  LRC_configNamespace cs[] = {
    {"default",{{"name", "", LRC_CHAR},{"xres", "", LRC_INT},{"yres", "", LRC_INT},
        {"method", "", LRC_INT},{"mrl", "", LRC_INT},{"module", "", LRC_CHAR},},6},
    {"logs", {{"dump", "", LRC_INT}}, 1}
  };
  allopts = 2;

  /* This is the easiest way to assign values ic cs[] */
  sprintf(cs[0].options[0].value,"%s",NAME_DEFAULT);
  sprintf(cs[0].options[1].value,"%d",XRES_DEFAULT);
  sprintf(cs[0].options[2].value,"%d",YRES_DEFAULT);
  sprintf(cs[0].options[3].value,"%d",METHOD_DEFAULT);
  sprintf(cs[0].options[4].value,"%d",MRL_DEFAULT);
  sprintf(cs[0].options[5].value,"%s",MODULE_DEFAULT);
  sprintf(cs[1].options[0].value,"%d",DUMP_DEFAULT);

  /* Assign allowed values */
  LRC_configTypes ct[] = {
    {"name", LRC_CHAR},
    {"xres", LRC_INT},
    {"yres", LRC_INT},
    {"method", LRC_INT},
    {"mrl", LRC_INT},
    {"module", LRC_CHAR},
    {"dump", LRC_INT}
  };
 
  int numCT = 7;

  /**
   * POPT CONFIG
   */
  struct poptOption mpi_poptHelpOptions[] = {
    { NULL, '\0', POPT_ARG_CALLBACK, (void *)mpi_displayUsage, 0, NULL, NULL },
    { "help", '?', 0, NULL, (int)'?', "Show this help message", NULL },
    { "usage", '\0', 0, NULL, (int)'u', "Display brief usage message", NULL },
      POPT_TABLEEND
    };
  
  struct poptOption cmdopts[] = {
    MPI_POPT_AUTOHELP
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
    {"dump", 'd', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.dump, 0,
    "how often write dump file [TODO]", "DUMP"},
    {"restart", 'r', POPT_ARG_VAL, &cd.restartmode, 1,
    "restart mode [TODO]","RESTART"},
    POPT_TABLEEND
  };
 
  /* MPI INIT */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  
  /**
   * BY DESIGN
   * Master cannot work alone.
   */
  if (mpi_size == 1){
     printf("You should have at least one slave!\n");
     MPI_Finalize();
     return 0;
  }

  /* Parse command line */
  poptContext poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);

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
  
  /* Restart mode set */
  if (cd.restartmode == 1){
    if(mpi_rank == 0) printf("TODO: restart mode\n");
     MPI_Finalize();
     return 0;
  }

  /* Config file set */
  if(strcmp(inifile,CONFIG_FILE_DEFAULT) != 0){
    configfile = 1;
  }

  if(mpi_rank == 0) welcome();

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
       MPI_Abort(MPI_COMM_WORLD, ERR_MPI);
    }

    printf("\n-> Mpifarm will use these startup values:\n\n");
    LRC_printAll(allopts,cs);
  }

  poptFreeContext(poptcon);
  
  /* HDF5 INIT */
  H5open();
  
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
  /* Create master datafile */
  file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  /* First of all, save configuration */
  writeConfig(file_id, allopts, cs);
  H5Fclose(file_id);

  /**
   * MPI CONFIG BCAST
   * Inform slaves what is all about.
   */

    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, MPI_DEST, MPI_COMM_WORLD);
  
  }else{
    buildDefaultConfigType(&cd, &defaultConfigType);
    MPI_Bcast(&cd, 1, defaultConfigType, MPI_DEST, MPI_COMM_WORLD);
  }
  
  /**
   * MODULE LOAD
   * If option -p is not set, use default.
   */
  sprintf(module_file, "mpifarm_module_%s.so", module_name);
 
  md.name = module_name;
  handler = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!handler){
    printf("Cannot load module '%s': %s\n", module_name, dlerror()); 
    MPI_Abort(MPI_COMM_WORLD, ERR_MODULE);
  }

  init = load_sym(handler,&md, "init", MODULE_ERROR);
  if(init) init(&md);

  query = load_sym(handler,&md, "query", MODULE_SILENT);
  if(query) query();

  /**
   * NOW, DO IT!
   */
  if(mpi_rank == 0) {
      master(handler, &md, &cd);
	} else {
      slave(handler, &md, &cd); 
  }

  cleanup = load_sym(handler, &md, "cleanup", MODULE_ERROR);
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
          VERSION, AUTHOR, EMAIL);

}
