/*
 * MECHANIC Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 * 
 * This file is part of MECHANIC code. 
 *
 * MECHANIC was created to help solving many numerical problems by providing tools
 * for improving scalability and functionality of the code. MECHANIC was released 
 * in belief it will be useful. If you are going to use this code, or its parts,
 * please consider referring to the authors either by the website or the user guide 
 * reference.
 *
 * http://mechanics.astri.umk.pl/projects/mechanic
 *
 * User guide should be provided with the package or 
 * http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
 *
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided 
 * that the following conditions are met:
 * 
 *  - Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *  - Neither the name of the Nicolaus Copernicus University nor the names of 
 *    its contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

/* 
 * @mainpage
 * @author Mariusz Slonina <mariusz.slonina@gmail.com>
 * with a little help of Kacper Kowalik <xarthisius.kk@gmail.com>
 *
 * @todo
 *   - HDF5 error handling with H5E (including file storage)
 *   - malloc error handling
 * 
 */
#include "mechanic.h"
#include "mechanic_internals.h"

/**
 * @page guide Quick Start Guide
 *
 */
int main(int argc, char *argv[]){  

  int allopts = 0; //number of namespaces read
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
  char checkpoint_path[MECHANIC_PATH];

  poptContext poptcon;
  int poptflags = 0;
  int error;
  int configfile = 0;

  int node;

  int mstat; //mechanic internal error value

  struct stat st; //stat.h
  
  module_query_int_f qd;

  hid_t file_id;

  configData cd; //struct for command line args
  moduleInfo md; //struct for module info

  #if HAVE_MPI_SUPPORT
    MPI_Datatype defaultConfigType;
  #endif

  /* Assign default config values */
  name = MECHANIC_NAME_DEFAULT;
  restartname = "";
  inifile = MECHANIC_CONFIG_FILE_DEFAULT;
  module_name = MECHANIC_MODULE_DEFAULT;
  
  sprintf(cd.name, "%s", MECHANIC_NAME_DEFAULT);
  sprintf(cd.datafile, "%s", MECHANIC_MASTER_FILE_DEFAULT); 
  sprintf(cd.module, "%s", MECHANIC_MODULE_DEFAULT);
  cd.xres = MECHANIC_XRES_DEFAULT;
  cd.yres = MECHANIC_YRES_DEFAULT;
  cd.method = MECHANIC_METHOD_DEFAULT;
  cd.checkpoint = MECHANIC_CHECKPOINT_DEFAULT;
  cd.restartmode = 0;
  cd.mode = MECHANIC_MODE_DEFAULT;
  cd.checkpoint_num = MECHANIC_CHECKPOINT_NUM_DEFAULT;

  LRC_configNamespace cs[] = {
    {"default",{
                 {"name", "", LRC_CHAR},
                 {"xres", "", LRC_INT},
                 {"yres", "", LRC_INT},
                 {"method", "", LRC_INT},
                 {"module", "", LRC_CHAR},
                 {"mode", "", LRC_INT},
               },
    6},
    {"logs", {
               {"checkpoint", "", LRC_INT},
               {"checkpoint_num", "", LRC_INT}
             }, 
    2}
  };
  allopts = 2;

  /* This is the easiest way to assign values ic cs[] */
  sprintf(cs[0].options[0].value,"%s",MECHANIC_NAME_DEFAULT);
  sprintf(cs[0].options[1].value,"%d",MECHANIC_XRES_DEFAULT);
  sprintf(cs[0].options[2].value,"%d",MECHANIC_YRES_DEFAULT);
  sprintf(cs[0].options[3].value,"%d",MECHANIC_METHOD_DEFAULT);
  sprintf(cs[0].options[4].value,"%s",MECHANIC_MODULE_DEFAULT);
  sprintf(cs[0].options[5].value,"%d",MECHANIC_MODE_DEFAULT);
  sprintf(cs[1].options[0].value,"%d",MECHANIC_CHECKPOINT_DEFAULT);
  sprintf(cs[1].options[1].value,"%d",MECHANIC_CHECKPOINT_NUM_DEFAULT);

  /* Assign allowed values */
  LRC_configTypes ct[] = {
    {"default", "name", LRC_CHAR},
    {"default", "xres", LRC_INT},
    {"default", "yres", LRC_INT},
    {"default", "method", LRC_INT},
    {"default", "module", LRC_CHAR},
    {"default", "mode", LRC_INT},
    {"logs", "checkpoint", LRC_INT},
    {"logs", "checkpoint_num", LRC_INT}
  };

  /* Number of allowed values */
  int numCT = 8;

  /**
   * POPT CONFIG
   */
  struct poptOption mechanic_poptHelpOptions[] = {
    { NULL, '\0', POPT_ARG_CALLBACK, (void *)mechanic_displayUsage, 0, NULL, NULL },
    { "help", '?', 0, NULL, (int)'?', "Show this help message", NULL },
    { "usage", '\0', 0, NULL, (int)'u', "Display brief usage message", NULL },
      POPT_TABLEEND
    };

  struct poptOption mechanic_poptModes[] = {
    {"masteralone", '0', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.mode, 0,
    "Masteralone",NULL},
    #if HAVE_MPI_SUPPORT
      {"farm", '1', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.mode, 1,
        "MPI task farm",NULL},
      {"multifarm", '2', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.mode, 2,
        "MPI multi task farm",NULL},
    #endif
    POPT_TABLEEND
  };

  struct poptOption mechanic_poptRestart[] = {
    {"restart", 'r', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.restartmode, 0,
      "Switch to restart mode", NULL},
    {"rpath", 'b', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &checkpoint_path, 0,
      "Path to checkpoint file", "/path/to/checkpoint/file"},
    POPT_TABLEEND
  };
  
  struct poptOption cmdopts[] = {
    {"name", 'n', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &name, 0,
    "Problem name", "NAME"},
    {"config", 'c', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &inifile, 0,
    "Config file", "CONFIG"},
    {"module", 'p', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &module_name, 0,
    "Module", "MODULE"},
    {"method", 'm', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.method, 0,
    "Pixel map method", "METHOD"},
    {"xres", 'x', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.xres, 0,
    "X resolution", "XRES"},
    {"yres", 'y', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.yres, 0,
    "Y resolution", "YRES"},
    {"checkpoint", 'd', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &cd.checkpoint, 0,
    "Checkpoint file write interval", "CHECKPOINT"},
    MECHANIC_POPT_MODES
    MECHANIC_POPT_RESTART
    MECHANIC_POPT_AUTOHELP
    POPT_TABLEEND
  };

  #if HAVE_MPI_SUPPORT
    // MPI INIT 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  #endif

  // HDF5 INIT 
  H5open();

  node = 0;
  #if HAVE_MPI_SUPPORT
    node = mpi_rank;
  #endif
  
  if(node == 0) welcome();

  // Parse command line 
  poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);

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
    if(node == 0){
      fprintf(stdout, "%s: %s\n", poptBadOption(poptcon, POPT_BADOPTION_NOALIAS), poptStrerror(optvalue));
      poptPrintHelp(poptcon, stdout, poptflags);
     }
     poptFreeContext(poptcon);
     mechanic_finalize(node);
     return 0;
  }
  
  /* Long help message set */
  if (help == 1){
    if(node == 0) poptPrintHelp(poptcon, stdout, 0);
    poptFreeContext(poptcon);
    mechanic_finalize(node);
    return 0;
  }
   
  /* Brief help message set */
  if (usage == 1){
    if(node == 0) poptPrintUsage(poptcon, stdout, 0);
    poptFreeContext(poptcon);
    mechanic_finalize(node);
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
/*
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
*/
  /* Config file set */
  if(strcmp(inifile, MECHANIC_CONFIG_FILE_DEFAULT) != 0 && cd.restartmode == 0){
    configfile = 1;
  }


  /* Reset options, we need to reuse them
   * to override values read from specified config file */
  if(node == 0){

    /* Read config file */
    allopts = readDefaultConfig(inifile, cs, ct, numCT, configfile);

    /**
     * Config file parsed. All read values becomes
     * new defaults 
     */
    mstat = assignConfigValues(allopts, &cd, cs, configfile, 0);

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
    mstat = assignConfigValues(allopts, &cd, cs, configfile, 1);
   
    /* Security check */
    if (cd.xres == 0 || cd.yres == 0){
       printf("X/Y map resolution should not be set to 0!\n");
       printf("If You want to do only one simulation, please set xres = 1, yres = 1\n");
       mechanic_abort(MECHANIC_ERR_SETUP);
    }

    printf("\n-> Mechanic will use these startup values:\n\n");
    LRC_printAll(allopts,cs);
  }

  poptFreeContext(poptcon);
 
  /**
   * @page modules MODULE LOAD
   *
   * @brief
   * If option -p is not set, use default.
   */
  sprintf(module_file, "mechanic_module_%s.so", module_name);
  
  md.name = module_name;
  handler = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!handler){
    printf("Cannot load module '%s': %s\n", module_name, dlerror()); 
    mechanic_abort(MECHANIC_ERR_MODULE);
  }

  init = load_sym(handler,&md, "init", MECHANIC_MODULE_ERROR);
  if(init) mstat = init(&md);

  query = load_sym(handler,&md, "query", MECHANIC_MODULE_SILENT);
  if(query) query();

 
  
  // Config file read
  if(node == 0){
    // Backup master data file.
    // If simulation was broken, and we are in restart mode
    // we don't have any master files, only checkpoint files.
    if(stat(cd.datafile,&st) == 0 && cd.restartmode == 0){
      sprintf(oldfile,"old-%s",cd.datafile);
      printf("-> File %s exists!\n", cd.datafile);
      printf("-> I will back it up for You now\n");
      printf("-> Backuped file: %s\n",oldfile);
      rename(cd.datafile,oldfile);
    }

 
   /**
    * @page storage HDF5 Storage Data Scheme
    *
    * We write data in the following scheme:
    * - /config -- configuration file
    * - /board -- map of computed pixels
    * - /data -- output data group
    * - /data/master -- master dataset
    *
    * Each slave can write own data files if needed.
    * In such case, please edit slaveIN/OUT functions in your module.
    *
    * @todo
    * Add logs functionality
    *
    */

  printf("MRL = %d\n", md.mrl);
  // Create master datafile
  file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  mstat = H5createMasterDataScheme(file_id, &md, &cd);

  // First of all, save configuration 
  LRC_writeHdfConfig(file_id, cs, allopts);
  H5Fclose(file_id);

  #if HAVE_MPI_SUPPORT
   // MPI CONFIG BCAST
   // Inform slaves what it is all about.
   if(cd.mode == 1 || cd.mode == 2){
    if(node == 0){
      mstat = buildDefaultConfigType(&cd, &defaultConfigType);
      MPI_Bcast(&cd, 1, defaultConfigType, MECHANIC_MPI_DEST, MPI_COMM_WORLD);
      MPI_Type_free(&defaultConfigType);
  
    }else{
      mstat = buildDefaultConfigType(&cd, &defaultConfigType);
      MPI_Bcast(&cd, 1, defaultConfigType, MECHANIC_MPI_DEST, MPI_COMM_WORLD);
      MPI_Type_free(&defaultConfigType);
    }
   }
  #endif
  }
  
  //Now load proper routines
  switch(cd.mode){
    case 0:
      mstat = mechanic_mode_masteralone(node, handler, &md, &cd);
      break;
    #if HAVE_MPI_SUPPORT
    case 1:
      mstat = mechanic_mode_farm(node, handler, &md, &cd);
      break;
    case 2:
      mstat = mechanic_mode_multifarm(node, handler, &md, &cd);
      break;
    #endif
    default:
      break;
  }

  //cleanup module
  cleanup = load_sym(handler, &md, "cleanup", MECHANIC_MODULE_ERROR);
  if(cleanup) mstat = cleanup();

  // MODULE UNLOAD
  dlclose(handler);
  
  // HDF5 FINALIZE 
  H5close();
	
  // FINALIZE 
  mechanic_finalize(node);

  return 0;
}

void welcome(){

  printf("MECHANIC %s\n\tauthor: %s\n\tbugs: %s\n\n",
          MECHANIC_VERSION, MECHANIC_AUTHOR, MECHANIC_EMAIL);

}
