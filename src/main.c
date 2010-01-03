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

int i = 0, j = 0, k = 0, opts = 0, n = 0;
    
/**
 * MAIN
 *
 */
int main(int argc, char *argv[]){  

  char* module_name;
  char* name;
  char module_file[MAX_VALUE_LENGTH];
  void* module;
  void* lib;
  char* dlresult;
  char optvalue;
  module_query_int_f qd;

  int restartmode = 0;
  int poptflags = 0;
  int error;
  int configfile = 0;

  hid_t file_id;

  configData cd; //struct for command line args

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

  configNamespace cs[] = {
    {"default",{{"name", ""},{"xres", ""},{"yres", ""},
        {"method", ""},{"mrl", ""},{"module", ""},},6},
    {"logs", {{"dump", ""}}, 1}
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
    {"restart", 'r', POPT_ARG_VAL, &restartmode, 1,
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
  if (restartmode == 1){
    if(mpi_rank == 0) printf("TODO: restart mode\n");
     MPI_Finalize();
     return 0;
  }

  /* Config file set */
  if(strcmp(inifile,CONFIG_FILE_DEFAULT) != 0){
    configfile = 1;
  }

  /* Reset options, we need to reuse them
   * to override values read from specified config file */
  if(mpi_rank == 0){
    
    welcome();

    /* Read config file. If the file is present,
     * all read values becomes new defaults. */
    allopts = readDefaultConfig(inifile, cs, configfile);
    assignConfigValues(allopts, &cd, cs, configfile, 0);

    /* Reset pointers */
    name = cd.name;
    module_name = cd.module;
  
    /* Override the vealues by command line args */
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

    lib = dlopen("libreadconfig.so", RTLD_NOW);
    if(!lib){
      printf("Cannot load libreadconfig: %s\n", dlerror()); 
      MPI_Abort(MPI_COMM_WORLD, ERR_OTHER);
    }

    qd = load_sym(lib,"printAll", MODULE_SILENT);
    printf("\n-> Mpifarm will use these startup values:\n\n");
    qd(allopts,cs);
    dlclose(lib);
  }

  poptFreeContext(poptcon);
  
  /* HDF5 INIT */
  H5open();
  
  /* Config file read */
  if(mpi_rank == 0){

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
   * If option -m is not set, use default.
   */
  sprintf(module_file, "mpifarm_module_%s.so", module_name);
  
  module = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!module){
    printf("Cannot load module '%s': %s\n", module_name, dlerror()); 
    MPI_Abort(MPI_COMM_WORLD, ERR_MODULE);
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
