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
 */

#include "mechanic.h"
#include "mechanic_internals.h"

/**
 * @page overview Overview
 * 
 * Handling numerical integrations is not a trivial task, either in one- or multi-cpu
 * environments. It can be a very stressfull job, especially when you deal with many sets
 * of initial conditions (like in many dynamical problems) and is full of human-based
 * mistakes. 
 *
 * Our main research is focused on studying dynamics of planetary systems, thus
 * it requires numerical job to be carefully done. Most of the problems can be coded by
 * hand, in fact, we did it in that way many times, however you can easily find that most
 * of these task are in some way repeatable.
 *
 * Let's have an example: We want to study the dynamics of a four body problem -- we have
 * a star, two massive planets and a big gap between them. We want to know the dynamical
 * behaviour (stability or not) of a test earth-like body in the gap. We can observe the
 * behaviour by using some values of semimajor axis and eccentricity of the small planet and
 * check the state of the system after some time. Then, we can change these values by a
 * small delta and observe the state of the system again. If we repeat it in some range of
 * semimajor axes and in some range of eccentricities we will get a dynamical map of the
 * planetary system (one can change semimajor axis and eccentricity by other orbital
 * elements, too). Each pixel of the map is a standalone numerical simulation which takes
 * some time on one cpu. 
 *
 * Now, the first approach is to use one cpu to do all the stuff. However, if computation
 * of the pixel lasts too long (especially when the configuration is quite stable), the
 * creation of the dynamical map is a very long process, and can take not one or two
 * weeks, but one or two months.
 *
 * We found another approach. Let's say, we have not one, but 10 cpus. If we can handle
 * sending initial conditions and receiving results, we can create the dynamical map of
 * the system at least 10 times faster! 
 *
 * And that's the reason we created @M. We needed some kind of a numerical interface
 * or framework that will handle our dynamical studies. We started by creating simple MPI
 * task farm model, however we realised quickly that using MPI framework can be useful not
 * only in image-based operations (dynamical map is a some kind of an image), but also in
 * many numerical problems with huge sets of initial conditions, or even tasks like
 * observations reductions, which lasts too long on single cpu. Thus we found that our
 * interface should handle such situations. 
 *
 * Now, @M is a multi-purpose numerical framework and interface. It is written in
 * ANSI C with help of MPI and HDF5 storage. It provides extensible user API and loadable
 * module support -- each numerical problem can be coded as a standalone module and loaded
 * dynamically during runtime. Mechanic uses LibReadConfig for handling configuration
 * and Popt library for commandline args.
 *
 * The latest snapshot can be grabbed from http://git.astri.umk.pl
 *
 * To teach you how to use @M as fast as possible, we decided to start this
 * guide with short crash course of using our software in real life, and then we
 * follow in details more advanced topics. Enjoy!
 */

/**
 * @page installation Installation
 * @M follows standard UNIX-like installation procedure.
 *
 * For compiling and running @M you need at least:
 *
 *  - MPI and HDF5 > 1.8
 *  - LibReadConfig with HDF5 support
 *  - Popt
 *  - C compiler
 *
 * To compile @M use following commands:
 * 
 * @verbatim
 * ./configure
 * make
 * make install
 * @endverbatim
 *
 * The standard installation includes example modules and documentation and requires MPI.
 * If you want to build @M on a single-cpu environment, use @c --disable-mpi switch of
 * @c configure. You can also disable example modules with @c --disable-examples and
 * documentation @c --disable-doc.
 *
 * If you want to create a module with Fortran code, remember to use proper Fortran
 * compiler. 
 *
 */

/**
 * @page quickstart Getting started
 * To fully understand what @M is, let's create a small C library, 
 * named "myhello", as follows:
 * @verbatim
 * #include <stdio.h>
 * #include "mechanic.h"
 * 
 * int myhello_init(moduleInfo* md){
 * 
 * 	md->author = "Mariusz Slonina"; 
 * 	md->date = "2010";
 * 	md->version = "1.0";
 * 	md->mrl = 3;
 * 
 * 	return 0;
 * }
 * 
 * int myhello_cleanup(moduleInfo* md){
 * 	return 0;
 * }
 * 
 * int myhello_pixelCompute(int node, moduleInfo* md, configData* d, masterData* r){
 *
 * 	r->res[0] = (double)r->coords[0];
 * 	r->res[1] = (double)r->coords[1];
 * 	r->res[2] = (double)r->coords[2];
 * 
 * 	return 0;
 * }
 *
 * int myhello_slave_out(int nodes, int node, moduleInfo* md, configData* d, masterData* r){
 * 	mechanic_message(MECHANIC_MESSAGE_INFO, "Hello from slave[%d]\n", node);
 *  return 0;
 * } @endverbatim
 *
 * Let's save it as @c mechanic_module_myhello.c. The prefix @c mechanic_module_
 * is important, as well as the name of the module -- as you can see, each
 * function must be prefixed with the name. 
 *
 * The first three functions: @c myhello_init(), @c myhello_cleanup() and @c
 * myhello_pixelCompute() are required for the module to work. @M will abort
 * if any of them is missing. The fourth one, @c hello_slave_out() is optional
 * and belongs to so-called themeable functions group.
 *
 * The @c myhello_init() function is called on module initialization and you need
 * to provide some information about the module, especially, @c md->mrl, which
 * is the length of the results array sended from the slave node to master node.
 *
 * The @c myhello_cleanup() function currently does nothing, however, it is
 * required for future development. 
 *
 * The @c myhello_pixelCompute() is the heart of your module. Here you can compute
 * any type of numerical problem, even call external applications. In this
 * simple example we just assign coordinates of the pixel on the map to the
 * result array.
 *
 * The @c myhello_slave_out() prints formatted message from the slave node on the screen.
 *
 * We need to compile this example to a shared library. We can do that by
 * calling
 * @verbatim
 * gcc -fPIC mechanic_module_myhello.c -o mechanic_module_myhello.o
 * gcc -shared mechanic_module_myhello.o -o mechanic_module_myhello.so @endverbatim
 *
 * However, @M need to know, where our module is, so we need to adjust @c
 * LD_LIBRARY_PATH accordingly.
 * 
 * We run @M by specifing
 * @verbatim
 * mpirun -np 3 mechanic -p myhello @endverbatim
 * This will run @M on three nodes, in task farm mode, with one master
 * node and two slaves.
 * @verbatim
 * -> Mechanic
 *  	v. 0.12-UNSTABLE-2
 *  	Author: MSlonina, TCfA, NCU
 *  	Bugs: mariusz.slonina@gmail.com
 *  	http://mechanics.astri.umk.pl/projects/mechanic
 * !! Config file not specified/doesn't exist. Will use defaults.
 * -> Mechanic will use these startup values:
 *
 * Namespace [default]:
 * name = mechanic [type 3]
 * xres = 5 [type 0]
 * yres = 5 [type 0]
 * method = 0 [type 0]
 * module = myhello [type 3]
 * mode = 1 [type 0]
 * 
 * Namespace [logs]:
 * checkpoint = 2000 [type 0]
 * checkpoint_num = 0 [type 0]
 * 
 * -> Hello from slave[1]
 * -> Hello from slave[2] @endverbatim
 *
 * Two last lines were printed using our simple module. In the working dir 
 * you should find also @c mechanic-master.h5 file. It is a data file written 
 * by the master node, and each run of Mechanic will produce such file. 
 * It containes all information about the setup of the simulation and data received from slaves. 
 * If you try
 * @verbatim
 * h5dump -n mechanic-master.h5 @endverbatim
 * you should see the following output:
 * @verbatim
 * HDF5 "mechanic-master.h5" {
 * FILE_CONTENTS {
 *  group      /
 *  dataset    /board
 *  group      /config
 *  dataset    /config/default
 *  dataset    /config/logs
 *  group      /data
 *  dataset    /data/master
 *  }
 * }@endverbatim
 * which describes the data storage in master file. 
 */
int main(int argc, char *argv[]){  

  int allopts = 0; //number of namespaces read
  
  char* module_name;
  char* name;
  char* restartname;
  char module_file[MECHANIC_FILE];
  char optvalue;
  void* handler;
  char oldfile[MECHANIC_FILE_OLD];
  char checkpoint_path[MECHANIC_PATH];

  poptContext poptcon;
  int poptflags = 0;
  int configfile = 0;

  int mpi_rank;
  int node;

  int mstat; //mechanic internal error value

  struct stat st; //stat.h
  
  hid_t file_id;

  configData cd; //struct for command line args
  moduleInfo md; //struct for module info

  MPI_Datatype defaultConfigType;

  // Assign default config values 
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

  // This is the easiest way to assign values ic cs[] 
  sprintf(cs[0].options[0].value,"%s",MECHANIC_NAME_DEFAULT);
  sprintf(cs[0].options[1].value,"%d",MECHANIC_XRES_DEFAULT);
  sprintf(cs[0].options[2].value,"%d",MECHANIC_YRES_DEFAULT);
  sprintf(cs[0].options[3].value,"%d",MECHANIC_METHOD_DEFAULT);
  sprintf(cs[0].options[4].value,"%s",MECHANIC_MODULE_DEFAULT);
  sprintf(cs[0].options[5].value,"%d",MECHANIC_MODE_DEFAULT);
  sprintf(cs[1].options[0].value,"%d",MECHANIC_CHECKPOINT_DEFAULT);
  sprintf(cs[1].options[1].value,"%d",MECHANIC_CHECKPOINT_NUM_DEFAULT);

  // Assign allowed values 
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

  // Number of allowed values 
  int numCT = 8;

  /**
   * @page setup Setup
   *
   * There are two ways to setup the computations: the config file and commandline args.
   * 
   * The setup steps are as follows:
   *
   *  - Read defaults
   *  - Read commandline args
   *  - If one of the args is @c -c or @c --config read config file 
   *  - Override config values according to the args
   *
   * If default config file is not present, Mechanic will use fixed defaults.
   * If option @-c is set, but the file doesn't exit, Mechanic will abort.
   *
   * The configuration is stored in master file, see storage.
   *
   * @section cli Commandline options
   * Mechanic uses the Popt library for handling commandline args. The possible arguments
   * are:
   *
   * - @c --help @c --usage @c -? -- prints help message
   * - @c --name @c -n  -- problem name
   * - @c --config @c -c -- config file
   * - @c --module @c -p -- module 
   * - @c --method @c -m -- method (0 -- default, 6 -- user-defined)
   * - @c --xres @c -x -- x resolution
   * - @c --yres @c -y -- y resolution
   * - @c --checkpoint @c -d -- checkpoint write interval
   *
   * Restart (checkpoints) options are:
   *
   * - @c --restart @c -r -- switch to restart mode
   * - @c --rpath @c -b -- checkpoint file path
   *
   * Mechanic can operate in different modes, see modes for detailes. You can switch
   * between them by using:
   *
   * - @c -0 -- masteralone mode
   * - @c -1 -- task farm mode
   * - @c -2 -- multi task farm mode
	 *
   * @section configfile Config file
   *
   * Mechanic uses LibReadConfig for handling config files. To load configuration from 
   * custom config file use @c -c switch. The sample config file is given below:
   *
   * @verbatim
	 * [default]
   * name= hello
   * xres = 4 #must be greater than 0
   * yres = 4 #must be greater than 0
   * method = 0 #single pixel -- 0, userdefined -- 6
   * module = echo
   * mode = 1
   *
   * [logs]
   * checkpoint=4
   * @endverbatim
   *
   * If any of variables is missing, Mechanic will use defaults. Namespaces are mandatory
   * and Mechanic will abort if missing. The abort will occur also if any other not-known
   * variable will be used in configuration file.
   *
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
    {"farm", '1', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.mode, 1,
    "MPI task farm",NULL},
    {"multifarm", '2', POPT_ARG_VAL|POPT_ARGFLAG_SHOW_DEFAULT, &cd.mode, 2,
    "MPI multi task farm",NULL},
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

  node = 0;
  // MPI INIT 
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  node = mpi_rank;

  // HDF5 INIT 
  H5open();
  //H5Eset_auto2(H5E_DEFAULT, H5error_handler, NULL);
  
  if(node == 0) welcome();

  // Parse command line 
  poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);
  optvalue = poptGetNextOpt(poptcon);
 
  // Bad option handling
  if (optvalue < -1){
    if(node == 0){
      mechanic_message(MECHANIC_MESSAGE_WARN, "%s: %s\n", 
					poptBadOption(poptcon, POPT_BADOPTION_NOALIAS), poptStrerror(optvalue));
      poptPrintHelp(poptcon, stdout, poptflags);
     }
     poptFreeContext(poptcon);
     mechanic_finalize(node);
     return 0;
  }
  
  // Long help message set
  if (help == 1){
    if(node == 0) poptPrintHelp(poptcon, stdout, 0);
    poptFreeContext(poptcon);
    mechanic_finalize(node);
    return 0;
  }
   
  // Brief help message set
  if (usage == 1){
    if(node == 0) poptPrintUsage(poptcon, stdout, 0);
    poptFreeContext(poptcon);
    mechanic_finalize(node);
    return 0;
  }

  /**
   * @page checkpoint Checkpoints
   *
   * TODO:
   * 1) -r should take problem name as an argument -- DONE
   * 2) check if master file exists
   *  - if exists, check if it's not corrupted (is hdf5) -- DONE
   *  - if it's corrupted, check for old copy
   *  - if the copy exists, check if it's not corrupted
   *  - if the copy is corrupted or does not exist, abort restart
   * 3) read config values from master file, ignore all command line opts etc.
   * 4) check board status
   *  - if board is full, abort, because we don't need restart completed simulation
   *  - if board is not full, check for empty pixels and compute them
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
  // Config file set
  if(strcmp(inifile, MECHANIC_CONFIG_FILE_DEFAULT) != 0 && cd.restartmode == 0){
    configfile = 1;
  }


  // Reset options, we need to reuse them
  // to override values read from specified config file
  if(node == 0){

    // Read config file
    allopts = readDefaultConfig(inifile, cs, ct, numCT, configfile);

    // Config file parsed. All read values becomes new defaults 
    mstat = assignConfigValues(allopts, &cd, cs, configfile, 0);

    // Reset pointers
    name = cd.name;
    module_name = cd.module;
  
    // Override the values by commandline args
    poptResetContext(poptcon);
    poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);
    optvalue = poptGetNextOpt(poptcon);
  
		// Security check: if mpi_size = 1 switch to masteralone mode
		if(mpi_size == 1){
			cd.mode = 0;
			mechanic_message(MECHANIC_MESSAGE_WARN, "MPI COMM SIZE = 1. Will switch to master alone mode now\n");
		}

    // In case of any commandline arg, we need to reassign config values.
    if(strcmp(name, cd.name) != 0) sprintf(cd.name,"%s",name);
    if(strcmp(module_name, cd.module) != 0) sprintf(cd.module,"%s",module_name);
    mstat = assignConfigValues(allopts, &cd, cs, configfile, 1);
   
    // Security check
    if (cd.xres == 0 || cd.yres == 0){
       mechanic_message(MECHANIC_MESSAGE_ERR, "X/Y map resolution should not be set to 0!\n");
       mechanic_message(MECHANIC_MESSAGE_ERR,"If You want to do only one simulation, please set xres = 1, yres = 1\n");
       mechanic_error(MECHANIC_ERR_SETUP);
    }

    mechanic_message(MECHANIC_MESSAGE_INFO,"Mechanic will use these startup values:\n\n");
    LRC_printAll(allopts,cs);
  }

  poptFreeContext(poptcon);
 
  /**
   * @page api UserAPI
   * @section modules 
   * The module interface allows user to load dynamically code with almost any type of
   * numerical problems. To load a module, use -p switch or "module" variable in the
   * config file. Otherwise, the default Echo module will be used.
   *
   */
  sprintf(module_file, "libmechanic_module_%s.so", module_name);
  
  md.name = module_name;
  handler = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
  if(!handler){
    mechanic_message(MECHANIC_MESSAGE_ERR, "Cannot load module '%s': %s\n", module_name, dlerror()); 
    mechanic_error(MECHANIC_ERR_MODULE);
  }

  init = load_sym(handler,&md, "init", "init", MECHANIC_MODULE_ERROR);
  if(init) mstat = init(&md);

  query = load_sym(handler,&md, "query", "query", MECHANIC_MODULE_SILENT);
  if(query) query(&md);

  // Config file has been read
  if(node == 0){
    // Backup master data file.
    // If simulation was broken, and we are in restart mode
    // we don't have any master files, only checkpoint files.
    if(stat(cd.datafile,&st) == 0 && cd.restartmode == 0){
      sprintf(oldfile,"old-%s",cd.datafile);
      mechanic_message(MECHANIC_MESSAGE_WARN, "File %s exists!\n", cd.datafile);
      mechanic_message(MECHANIC_MESSAGE_WARN, "I will back it up for You now\n");
      mechanic_message(MECHANIC_MESSAGE_WARN, "Backuped file: %s\n",oldfile);
      rename(cd.datafile,oldfile);
    }
 
   /**
    * @page storage Storage
    *
    * We write data in the following scheme:
    *  - /config -- configuration file
    *  - /board -- map of computed pixels
    *  - /data -- output data group
    *  - /data/master -- master dataset
    *
    * Each slave can write own data files if needed.
    * In such case, please edit slaveIN/OUT functions in your module.
    *
    * @todo
    * Add logs functionality
    *
    */

  // Create master datafile
  file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  mstat = H5createMasterDataScheme(file_id, &md, &cd);

  // First of all, save configuration 
  LRC_writeHdfConfig(file_id, cs, allopts);
  H5Fclose(file_id);

  }
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
  
  // Now load proper routines
  switch(cd.mode){
    case 0:
      mstat = mechanic_mode_masteralone(node, handler, &md, &cd);
      break;
    case 1:
      mstat = mechanic_mode_farm(node, handler, &md, &cd);
      break;
    case 2:
      mstat = mechanic_mode_multifarm(node, handler, &md, &cd);
      break;
    default:
      break;
  }

  // Module cleanup
  cleanup = load_sym(handler, &md, "cleanup", "cleanup", MECHANIC_MODULE_ERROR);
  if(cleanup) mstat = cleanup(&md);

  // Module unload
  dlclose(handler);
  
  // HDF5 finalize 
  H5close();
	
  // Finalize
  mechanic_finalize(node);

  return 0;
}

/** 
 * @page troubleshooting Troubleshooting
 * Some known bugs. 
 */

void welcome(){

  mechanic_message(MECHANIC_MESSAGE_INFO, "%s\n", MECHANIC_NAME);
  mechanic_message(MECHANIC_MESSAGE_CONT, "v. %s\n", MECHANIC_VERSION);
  mechanic_message(MECHANIC_MESSAGE_CONT, "Author: %s\n", MECHANIC_AUTHOR);
  mechanic_message(MECHANIC_MESSAGE_CONT, "Bugs: %s\n", MECHANIC_BUGREPORT);
  mechanic_message(MECHANIC_MESSAGE_CONT, "%s\n", MECHANIC_URL);

}
