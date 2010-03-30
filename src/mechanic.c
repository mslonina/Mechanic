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
  
  /* POPT Helpers */
  char* module_name;
  char* name;
  /*char* restartname;*/
  char checkpoint_path[MECHANIC_MAXLENGTH];
  int mode = 0;
  int xres = 0;
  int yres = 0;
  int method = 0;
  int checkpoint = 0;
  int poptflags = 0, configfile = 0, restartmode = 0;
  char optvalue;
  char convstr[MECHANIC_MAXLENGTH];
  void* handler;
  poptContext poptcon;

  char module_file[MECHANIC_MAXLENGTH];
  char* oldfile;
  size_t olen, opreflen, dlen;
  
  /* HDF Helpers */
  hid_t file_id;

  /* MECHANIC Helpers */
  configData cd; /* struct for command line args */
  moduleInfo md; /* struct for module info */
  int mstat; /* mechanic internal error value */
  struct stat st; /* stat.h */

  /* MPI Helpers */
  int mpi_rank;
  int node = 0;
  MPI_Status mpi_status;
  int lengths[4];
  int i = 0;
  size_t slen;
  char pack_buffer[MECHANIC_MAXLENGTH];
  int pack_position;
	
  /* LRC defaults */
  LRC_configDefaults cs[] = {
    {"default", "name", MECHANIC_NAME_DEFAULT, LRC_STRING},
		{"default", "xres", MECHANIC_XRES_DEFAULT, LRC_INT},
    {"default", "yres", MECHANIC_YRES_DEFAULT, LRC_INT},
    {"default", "method", MECHANIC_METHOD_DEFAULT, LRC_INT},
    {"default", "module", MECHANIC_MODULE_DEFAULT, LRC_STRING},
    {"default", "mode", MECHANIC_MODE_DEFAULT, LRC_INT},
    {"logs", "checkpoint", MECHANIC_CHECKPOINT_DEFAULT, LRC_INT},
		LRC_OPTIONS_END
  };

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
    {"masteralone", '0', POPT_ARG_VAL, &mode, 0, "Masteralone",NULL},
    {"farm", '1', POPT_ARG_VAL, &mode, 1, "MPI task farm",NULL},
    {"multifarm", '2', POPT_ARG_VAL, &mode, 2, "MPI multi task farm",NULL},
    POPT_TABLEEND
  };

  struct poptOption mechanic_poptRestart[] = {
    {"restart", 'r', POPT_ARG_VAL, &restartmode, 0, "Switch to restart mode", NULL},
    {"rpath", 'b', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &checkpoint_path, 0, "Path to checkpoint file", "/path/to/checkpoint/file"},
    POPT_TABLEEND
  };

  struct poptOption mechanic_poptDebug[] = {
    {"debug", 'z', POPT_ARG_VAL, &debug, 1, "Debug mode", NULL},
    {"silent", 's', POPT_ARG_VAL, &silent, 1, "Silent mode", NULL},
    POPT_TABLEEND
  };
  
  struct poptOption cmdopts[] = {
    {"name", 'n', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &name, 0, "Problem name", "NAME"},
    {"config", 'c', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &inifile, 0, "Config file", "CONFIG"},
    {"module", 'p', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &module_name, 0, "Module", "MODULE"},
    {"method", 'm', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &method, 0, "Pixel map method", "METHOD"},
    {"xres", 'x', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &xres, 0, "X resolution", "XRES"},
    {"yres", 'y', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &yres, 0, "Y resolution", "YRES"},
    {"checkpoint", 'd', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT, &checkpoint, 0, "Checkpoint file write interval", "CHECKPOINT"},
    MECHANIC_POPT_MODES
    MECHANIC_POPT_RESTART
    MECHANIC_POPT_DEBUG
    MECHANIC_POPT_AUTOHELP
    POPT_TABLEEND
  };

  /* MPI INIT */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  node = mpi_rank;
	
	mechanic_message(MECHANIC_MESSAGE_DEBUG, "MPI started\n");

  /* HDF5 INIT */
  H5open();
  /* H5Eset_auto2(H5E_DEFAULT, H5error_handler, NULL); */  
  
  if(node == 0) welcome();

  /* CONFIGURATION */
  if(node == 0){
    
    /* Assign LRC defaults */
 	  LRC_assignDefaults(cs);	
	  mechanic_message(MECHANIC_MESSAGE_DEBUG, "LRC defaults assigned\n");

    /* Assign POPT defaults */
    name = MECHANIC_NAME_DEFAULT;
    module_name = MECHANIC_MODULE_DEFAULT;
    inifile = MECHANIC_CONFIG_FILE_DEFAULT;
    xres = atoi(MECHANIC_XRES_DEFAULT);
    yres = atoi(MECHANIC_YRES_DEFAULT);
    method = atoi(MECHANIC_METHOD_DEFAULT);
    checkpoint = atoi(MECHANIC_CHECKPOINT_DEFAULT);
    mode = atoi(MECHANIC_MODE_DEFAULT);

  } 
    /* Parse command line */
    poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);
    optvalue = poptGetNextOpt(poptcon);
 
    /* Bad option handling */
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
  
    /* Long help message set */
    if (help == 1){
      if(node ==0){
        poptPrintHelp(poptcon, stdout, 0);
      }
      poptFreeContext(poptcon);
      mechanic_finalize(node);
      return 0;
    }
   
    /* Brief help message set */
    if (usage == 1){
      if(node == 0){
        poptPrintUsage(poptcon, stdout, 0);
      }
      poptFreeContext(poptcon);
      mechanic_finalize(node);
      return 0;

    }

  if(node == 0){
  /* Process options */
  
  /* Config file is set */
   if(strcmp(inifile, MECHANIC_CONFIG_FILE_DEFAULT) != 0 && restartmode == 0) configfile = 1;

    /* Step1a: Read config file, if any.
     * This will override LRC_configDefaults */
    allopts = readDefaultConfig(inifile, configfile);

    /* Step1b: Reset POPT defaults 
     * We need to reassign popt defaults in case of config file, if any.
     * If there was no config file, defaults will be untouched. */
    name = LRC_getOptionValue("default","name");
    module_name = LRC_getOptionValue("default","module");
    mode = LRC_option2int("default","mode");
    xres = LRC_option2int("default","xres");
    yres = LRC_option2int("default","yres");
    method = LRC_option2int("default", "method");
    checkpoint = LRC_option2int("logs","checkpoint");

    /* Step2a: Read commandline options, if any.
     * This will override popt defaults. */
    poptResetContext(poptcon);
    poptcon = poptGetContext (NULL, argc, (const char **) argv, cmdopts, 0);
    optvalue = poptGetNextOpt(poptcon);

    /* Step2b: Modify options by commandline 
     * If there was no commandline options, LRC table will be untouched. 
     * */
    LRC_modifyOption("default", "name", name, LRC_STRING);
    LRC_modifyOption("default", "module", module_name, LRC_STRING);

    LRC_itoa(convstr, xres, LRC_INT);
		LRC_modifyOption("default", "xres", convstr, LRC_INT);

    LRC_itoa(convstr, yres, LRC_INT);
    LRC_modifyOption("default", "yres", convstr, LRC_INT);
    
    LRC_itoa(convstr, method, LRC_INT);
    LRC_modifyOption("default", "method", convstr, LRC_INT);
    
    LRC_itoa(convstr, mode, LRC_INT);
    LRC_modifyOption("default", "mode", convstr, LRC_INT);
    
    LRC_itoa(convstr, checkpoint, LRC_INT);
    LRC_modifyOption("logs", "checkpoint", convstr, LRC_INT);

    /* Step3: Options are processed, we can now assign config values. */
    mstat = assignConfigValues(&cd);
 
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"Config file contents:\n\n");
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"name: %s\n", cd.name);
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"datafile: %s\n", cd.datafile);
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"module: %s\n", cd.module);
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"res[%d, %d]\n", cd.xres, cd.yres);
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"mode: %d\n", cd.mode);
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"method: %d\n", cd.method);
    mechanic_message(MECHANIC_MESSAGE_DEBUG,"checkpoint: %d\n", cd.checkpoint);

    /* Security check: if mpi_size = 1 switch to masteralone mode */
		if(mpi_size == 1){
			cd.mode = 0;
			mechanic_message(MECHANIC_MESSAGE_WARN, "MPI COMM SIZE = 1. Will switch to master alone mode now\n");
		}
  
    /* Security check */
    if (cd.xres == 0 || cd.yres == 0){
       mechanic_message(MECHANIC_MESSAGE_ERR, "X/Y map resolution should not be set to 0!\n");
       mechanic_message(MECHANIC_MESSAGE_ERR,"If You want to do only one simulation, please set xres = 1, yres = 1\n");
       mechanic_error(MECHANIC_ERR_SETUP);
    }

    mechanic_message(MECHANIC_MESSAGE_INFO,"Mechanic will use these startup values:\n\n");
    if(silent == 0) LRC_printAll(allopts,cs);

    /* Backup master data file.
     * If simulation was broken, and we are in restart mode
     * we don't have any master files, only checkpoint files.*/
    if(stat(cd.datafile,&st) == 0 && cd.restartmode == 0){
      
      opreflen = strlen(MECHANIC_FILE_OLD_PREFIX);
      dlen = strlen(cd.datafile);
      olen = dlen + opreflen + 2*sizeof(char*);
      
      /* Allocate memory for the backup name */
      oldfile = malloc(olen + sizeof(char*));
      if(oldfile == NULL) mechanic_error(MECHANIC_ERR_MEM);

      strncpy(oldfile, MECHANIC_FILE_OLD_PREFIX, opreflen);
      oldfile[opreflen] = LRC_NULL;

      strncat(oldfile, cd.datafile, dlen);
      oldfile[olen] = LRC_NULL;

      mechanic_message(MECHANIC_MESSAGE_WARN, "File %s exists!\n", cd.datafile);
      mechanic_message(MECHANIC_MESSAGE_WARN, "I will back it up for You now\n");
      mechanic_message(MECHANIC_MESSAGE_WARN, "Backuped file: %s\n",oldfile);

      /* Now we can savely rename files */
      rename(cd.datafile,oldfile);
      free(oldfile);
    }
  }
 /* CONFIGURATION END */

  /* Send standby message to slaves
   * This is because slaves don't know anything about the run until master node
   * send the configuration with bcast. Without this part, code will raise some
   * segfaults and endless-loops. 
   * The standby message containes the mode in which we are working.
   * */
  if(mpi_size > 1){
    if(node == 0){
      for(i = 1; i < mpi_size; i++){
        if(cd.mode == MECHANIC_MODE_MASTERALONE){
          mechanic_message(MECHANIC_MESSAGE_WARN, "Terminating SLAVE[%d]\n",i);
          MPI_Send(&cd.mode, 1, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
        }else{
          MPI_Send(&cd.mode, 1, MPI_INT, i, MECHANIC_MPI_STANDBY_TAG, MPI_COMM_WORLD);
        }
      }
    }else{
      MPI_Recv(&cd.mode, 1, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      if(mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG){
        mechanic_finalize(node);
        return 0;
      }
    }
  }

 /* MPI CONFIGURATION BCAST
  * Inform slaves what it is all about. */
   if(cd.mode != MECHANIC_MODE_MASTERALONE){

     /* Send to slaves information about lengths of important strings */
      if(node == 0){
      
        lengths[0] = (int)strlen(cd.name);
        lengths[1] = (int)strlen(cd.datafile);
        lengths[2] = (int)strlen(cd.module);

        mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node[%d] lengths[%d, %d, %d]\n", node, lengths[0], lengths[1], lengths[2]);

        for(i = 1; i < mpi_size; i++){
          MPI_Send(&lengths, 3, MPI_INT, i, MECHANIC_MPI_STANDBY_TAG, MPI_COMM_WORLD);
        }
  
      }else{

        mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node[%d] ready\n", node);

        /* Slave node doesn't read config file, so we need to receive first
         * information about lengths of string data */

        MPI_Recv(&lengths, 3, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
        mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node[%d] lengths[%d, %d, %d]\n", node, lengths[0], lengths[1], lengths[2]);

        /* Now we have to allocate memory for cd struct */
        slen = (size_t)lengths[0];
        cd.name = malloc(slen + sizeof(char*));
        if(cd.name == NULL) mechanic_error(MECHANIC_ERR_MEM);
      
        slen = (size_t)lengths[1];
        cd.datafile = malloc(slen + sizeof(char*));
        if(cd.datafile == NULL) mechanic_error(MECHANIC_ERR_MEM);

        slen = (size_t)lengths[2];
        cd.module = malloc(slen + sizeof(char*));
        if(cd.module == NULL) mechanic_error(MECHANIC_ERR_MEM);
      
      }

      if(node == 0){

        pack_position = 0;

        MPI_Pack(cd.name, lengths[0], MPI_CHAR, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(cd.datafile, lengths[1], MPI_CHAR, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(cd.module, lengths[2], MPI_CHAR, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(&cd.xres, 1, MPI_INT, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(&cd.yres, 1, MPI_INT, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(&cd.method, 1, MPI_INT, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(&cd.checkpoint, 1, MPI_INT, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(&cd.restartmode, 1, MPI_INT, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);
        MPI_Pack(&cd.mode, 1, MPI_INT, pack_buffer, MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Bcast(pack_buffer, MECHANIC_MAXLENGTH, MPI_PACKED, MECHANIC_MPI_DEST, MPI_COMM_WORLD);

      }else{

        /* Receive data from master node */
        pack_position = 0;
        MPI_Bcast(pack_buffer, MECHANIC_MAXLENGTH, MPI_PACKED, MECHANIC_MPI_DEST, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, cd.name, lengths[0], MPI_CHAR, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, cd.datafile, lengths[1], MPI_CHAR, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, cd.module, lengths[2], MPI_CHAR, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, &cd.xres, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, &cd.yres, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, &cd.method, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, &cd.checkpoint, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, &cd.restartmode, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position, &cd.mode, 1, MPI_INT, MPI_COMM_WORLD);

        mechanic_message(MECHANIC_MESSAGE_DEBUG,"Node [%d] received following configuration:\n\n", node);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"name: %s\n", cd.name);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"datafile: %s\n", cd.datafile);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"module: %s\n", cd.module);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"res[%d, %d]\n", cd.xres, cd.yres);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"mode: %d\n", cd.mode);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"method: %d\n", cd.method);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,"checkpoint: %d\n", cd.checkpoint);

      }
 
   }

  /**
   * @page api UserAPI
   * @section modules 
   * The module interface allows user to load dynamically code with almost any type of
   * numerical problems. To load a module, use -p switch or "module" variable in the
   * config file. Otherwise, the default Echo module will be used.
   *
   */

    sprintf(module_file, "libmechanic_module_%s.so", cd.module);
  
    md.name = cd.module;
    handler = dlopen(module_file, RTLD_NOW|RTLD_GLOBAL);
    if(!handler){
      mechanic_message(MECHANIC_MESSAGE_ERR, "Cannot load module '%s': %s\n", cd.module, dlerror()); 
      mechanic_error(MECHANIC_ERR_MODULE);
    }
  
   /* Module init */
    init = load_sym(handler,&md, "init", "init", MECHANIC_MODULE_ERROR);
    if(init) mstat = init(&md);

    /* Module query */
    query = load_sym(handler,&md, "query", "query", MECHANIC_MODULE_SILENT);
    if(query) query(&md);

 /* }*/

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
  
  /* There are some special data in module,
   * thus we can create master data file after module has been successfully
   * loaded. */
  if(node == 0){
    /* Create master datafile */
    file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    mstat = H5createMasterDataScheme(file_id, &md, &cd);

    /* First of all, save configuration */
    LRC_HDF5Writer(file_id);
    H5Fclose(file_id);
  }
  
  /* Now load proper routines */
  switch(cd.mode){
    case MECHANIC_MODE_MASTERALONE:
      mstat = mechanic_mode_masteralone(node, handler, &md, &cd);
      break;
    case MECHANIC_MODE_FARM:
      mstat = mechanic_mode_farm(node, handler, &md, &cd);
      break;
    case MECHANIC_MODE_MULTIFARM:
      mstat = mechanic_mode_multifarm(node, handler, &md, &cd);
      break;
    default:
      break;
  }

  /* Module cleanup */
    cleanup = load_sym(handler, &md, "cleanup", "cleanup", MECHANIC_MODULE_ERROR);
    if(cleanup) mstat = cleanup(&md);

  /* Free POPT */
  if(node == 0) poptFreeContext(poptcon);

  /* Mechanic cleanup */
   
    free(cd.name);
    free(cd.datafile);
    free(cd.module);

    /* Module unload */
    dlclose(handler);

  /* HDF5 finalize */
  H5close();

	/* Cleanup LRC */
  if(node == 0)	LRC_cleanup();
	
  /* Finalize */
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
