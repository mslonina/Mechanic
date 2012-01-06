/*
 * MECHANIC
 *
 * Copyright (c) 2010-2011, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 *
 * This file is part of MECHANIC code.
 *
 * MECHANIC was created to help solving many numerical problems by providing
 * tools for improving scalability and functionality of the code. MECHANIC was
 * released in belief it will be useful. If you are going to use this code, or
 * its parts, please consider referring to the authors either by the website
 * or the user guide reference.
 *
 * http://git.astri.umk.pl/projects/mechanic
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Nicolaus Copernicus University nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * @mainpage
 * @author Mariusz Slonina <mariusz.slonina@gmail.com>
 * @version 0.12
 * @date 2011
 *
 * @todo
 *   - HDF5 error handling with H5E (including file storage)
 *   - multifarm mode
 *
 */

#include "mechanic.h"
#include "mechanic_internals.h"

/**
 * @page intro Introduction
 *
 * Handling numerical simulations is not a trivial task, either in one- or
 * multi-cpu environments. It can be a very stressfull job, especially when
 * you deal with many sets of initial conditions (like in many dynamical
 * problems) and is full of human-based mistakes.
 *
 * Our main research is focused on studying dynamics of planetary systems,
 * thus it requires numerical job to be carefully done. Most of the problems
 * can be coded by hand, in fact, we did it in that way many times, however
 * you can easily find that most of these task are in some way, or in some
 * part, repeatable.
 *
 * Let's have an example: We want to study the dynamics of a four body problem
 * -- we have a star, two massive planets and a big gap between them. We want
 * to know the dynamical behaviour (stability or not) of a test earth-like
 * body in the gap. We can observe the behaviour by using some values of
 * semimajor axis and eccentricity of the small planet and check the state of
 * the system after some time. Then, we can change these values by a small
 * delta and observe the state of the system again. If we repeat it in some
 * range of semimajor axes and in some range of eccentricities we will get
 * a dynamical map of the planetary system (one can exchange semimajor axis
 * and eccentricity by other orbital elements, too). Each pixel of the map is
 * a standalone numerical simulation which takes some time on one cpu.
 *
 * Now, the first approach is to use one cpu to do all the stuff. However, if
 * computation of the pixel lasts too long (especially when the configuration
 * is quite stable), the creation of the dynamical map is a very long process,
 * and can take not one or two weeks, but one or two months.
 *
 * There is a second approach. Let's say, we have not one, but 10 cpus. If we
 * can handle sending initial conditions and receiving results, we can create
 * the dynamical map of the system at least 10 times faster!
 *
 * And that's the reason we created @M. We needed some kind of a numerical
 * interface or framework that will handle our dynamical studies. We started
 * by creating simple MPI Task farm model, however we quickly realised that
 * using MPI framework can be useful not only in image-based operations
 * (dynamical map is a some kind of an image), but also in many numerical
 * problems with huge sets of initial conditions, or even tasks like
 * observations reductions, which lasts too long on single cpu. We found
 * that our interface should handle such situations, too.
 *
 * Now, @M is a multi-purpose numerical framework and interface. It is written
 * in C99 with help of MPI and HDF5 storage. It provides extensible
 * user API and loadable module support -- each numerical problem
 * can be coded as a standalone module, loaded dynamically during runtime.
 * @M uses @c LibReadConfig (@c LRC) for handling configuration aspects
 * and @c Popt library for command line (@c CLI) options.
 *
 * @M is in pre-alpha stage, this means, that there are some bugs in code,
 * some parts are not finished, and some features are not implemented yet.
 * However, we try to keep the @c Master branch as stable and useful as
 * possible. Feel free to participate in the development, test the software
 * and send bugs. The latest snapshot can be grabbed from
 * http://git.astri.umk.pl. 
 *
 * @M is distributed under terms od BSD license. This means you can use our
 * software both for personal and commercial stuff. We released the code
 * to the public, because we believe, that the science and its tools should
 * be open for everyone. If you find @M useful for your research, we will be
 * appreciated if you refer to this user guide and our project homepage:
 * http://mechanics.astri.umk.pl/project/mechanic.
 *
 * In this userguide, we assume you have some basic knowledge on
 * C-programming and using Unix-shell.
 */

/**
 * @page install Installation
 * 
 * @section requirements Requirements
 * Following requirements have to be met to compile the @M:
 *
 * - @c MPI2 implementation (we prefer @c OpenMPI, and @M was tested with it)
 * - @c HDF5, at least 1.8
 * - @c LibReadConfig with @c HDF5 support -- @c LRC can be downloaded from
 *   our git repository, since it is a helper tool builded especially for @M,
 *   but can be used independly. By default it comes with HDF5 support. 
 * - @c Popt library (may be already installed on your system)
 * - C compiler (we recommend at least @c gcc 4.6)
 * - CMake, at least 2.8
 *
 * To download the latest snapshot of @M try
 * @code
 * http://git.astri.umk.pl/project/mechanic
 * @endcode
 *
 * @section compilation Compilation 
 *
 * @code
 * tar -xvvf mechanic-VERSION.tar.gz
 * cd mechanic-VERSION
 * mkdir build
 * CC=mpicc FC=mpif90 cmake ..
 * make
 * sudo make install
 * @endcode
 *
 * By default @M comes with Fortran support. To disable it, pass @c
 * -DBUILD_FORTRAN:BOOL=OFF to cmake. Default installation path points to @c /usr/local .
 *  You can change this setting with @c -DCMAKE_INSTALL_PREFIX=/your/custom/path . 
 *
 * Altought @M requires @c MPI, it can be runned in a single-cpu environments
 * (we call it "fake-MPI"). @M should do its job both on 32 and 64-bits
 * architectures with *nix-like system on board.
 */

/* We build a custom tag-system to override and extend some Doxygen
 * functionality. The parser searches for @idoc and @icode commands, takes two
 * arguments: file name and tag to include block of documentation. The output
 * is an input file for Doxygen. Thus, we can quite easily create not only a
 * reference guide for developers, but different sets of additional
 * documentation, i.e the user guide.
 *
 * Blocks of code, used in the documentation are marked as follows:
 * [TAG] (in comment block)
 * code
 * code
 * [/TAG] (in comment block)
 *
 * The additional tag system was created because of lack of Doxygen support
 * for including specific blocks of documentation, and should be switched to
 * Doxygen solution, if there will be any.
 *
 * If you will try run Doxygen without using the parser, you will notice
 * errors on missing @idoc and @icode commands. To get rid of it, just create
 * aliases in standard Doxygen way, i.e.
 *
 * @idoc=@c
 * @icode=@c
 *
 */

/**
 * @page quickstart Getting Started
 * @idoc modules/hello/mechanic_module_hello.c HELLO
 * @idoc core/mechanic_core_setup.c SETUP
 * @idoc core/mechanic_core_hdf5.c STORAGE
 * @idoc core/mechanic_core_checkpoint.c CHECKPOINT
 *
 * @page advanced Advanced Topics
 * @idoc modules/mechanic_module_module.c TEMPLATES
 * @idoc modules/echo/mechanic_module_echo.c ECHO
 * @idoc modules/mandelbrot/mechanic_module_mandelbrot.c MANDELBROT
 * @idoc modules/mechanic_module_module.c METHOD6
 * @idoc core/mechanic_core_hdf5.c DATA
 * @idoc libs/orbit/mechanic_lib_orbit.c ORBIT
 * @idoc fortran/mechanic_fortran.f03 F2003BIND_REF
 *
 * @page devel Short Developer's Guide
 * @idoc core/mechanic_core_devel.c DEVEL
 *
 * @page troubleshooting Troubleshooting
 * @idoc core/mechanic_core_errors.c ERRORS
 *
 */

int main(int argc, char** argv) {

  /* MECHANIC Helpers */
  Config cd; /* struct for command line args */
  Module md; /* struct for module info */
  int i, n, mstat; /* mechanic internal error value */
  struct stat st; /* stat.h */
  struct stat ct; /* stat.h */
  mechanic_internals internals;
  char* oldfile;
  size_t olen, opreflen, dlen;

  /* LRC Helpers */
  LRC_configNamespace* head = NULL;
  LRC_configNamespace* module_head = NULL;
  LRC_MPIStruct* ccc = NULL;

  /* POPT Helpers */
  char* module_name;
  char* name;
  int mode = 0;
  /*unsigned long*/ int xres = 0;
  /*unsigned long*/ int yres = 0;
  /*unsigned long*/ int checkpoint = 0;
  int poptflags = 0;
  int useConfigFile = 0;
  int useModuleConfigFile = 0;
  char optvalue;
  int restartmode = 0;
  char convstr[MECHANIC_MAXLENGTH];
  poptContext poptcon;
  help = 0;
  usage = 0;

  /* Module functions */
  module_query_int_f init, schema, query, cleanup;

  /* HDF Helpers */
  hid_t file_id;

  /* MPI Helpers */
  int mpi_rank;
  int mpi_size;
  int node = 0;
  int lengths[4];
  MPI_Datatype ConfigType, lrc_mpi_t;
  MPI_Status mpi_status;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int processor_len;

  /* MPI INIT */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  node = mpi_rank;

  MPI_Get_processor_name(processor_name, &processor_len);

  /* LRC defaults */
  LRC_configDefaults cs[] = {
    {"default", "name", MECHANIC_NAME_DEFAULT, LRC_STRING},
    {"default", "xres", MECHANIC_XRES_DEFAULT, LRC_INT},
    {"default", "yres", MECHANIC_YRES_DEFAULT, LRC_INT},
    {"default", "module", MECHANIC_MODULE_DEFAULT, LRC_STRING},
    {"default", "mconfig", MECHANIC_CONFIG_FILE_DEFAULT, LRC_STRING},
    {"default", "mode", MECHANIC_MODE_DEFAULT, LRC_INT},
    {"logs", "checkpoint", MECHANIC_CHECKPOINT_DEFAULT, LRC_INT},
    LRC_OPTIONS_END
  };

  /* POPT tables */
  struct poptOption mechanic_poptHelpOptions[] = {
    {"help", '?', POPT_ARG_VAL, &help, 1,
      "Show this help message", NULL},
    {"usage", '\0', POPT_ARG_VAL, &usage, 1,
      "Display brief usage message", NULL},
      POPT_TABLEEND
    };

  struct poptOption mechanic_poptModes[] = {
    {"masteralone", MECHANIC_MODE_MASTERALONE_P, POPT_ARG_VAL, &mode,
      MECHANIC_MODE_MASTERALONE, "Masteralone", NULL},
    {"farm", MECHANIC_MODE_FARM_P, POPT_ARG_VAL, &mode,
      MECHANIC_MODE_FARM, "MPI task farm", NULL},
    POPT_TABLEEND
  };

  struct poptOption mechanic_poptRestart[] = {
    {"restart", 'r', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT, &CheckpointFile,
      0, "Path to checkpoint file", "/path/to/checkpoint/file"},
    POPT_TABLEEND
  };

  struct poptOption mechanic_poptDebug[] = {
    {"debug", 'g', POPT_ARG_VAL, &debug, 1, "Debug mode", NULL},
    {"silent", 's', POPT_ARG_VAL, &silent, 1, "Silent mode", NULL},
    POPT_TABLEEND
  };

  struct poptOption cmdopts[] = {
    {"name", 'n', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
      &name, 0, "Problem name", "NAME"},
    {"config", 'c', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
      &ConfigFile, 0, "Config file", "/path/to/config/file"},
    {"module", 'p', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
      &module_name, 0, "Module", "MODULE"},
    {"mconfig", 'm', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
      &ModuleConfigFile, 0, "Module Config file", "/path/to/module/config/file"},
    {"xres", 'x', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
      &xres, 0, "X resolution", "XRES"},
    {"yres", 'y', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
      &yres, 0, "Y resolution", "YRES"},
    {"checkpoint", 'd', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
      &checkpoint, 0, "Checkpoint file write interval", "CHECKPOINT"},
    MECHANIC_POPT_MODES
    MECHANIC_POPT_RESTART
    MECHANIC_POPT_DEBUG
    MECHANIC_POPT_AUTOHELP
    POPT_TABLEEND
  };

  if (node == MECHANIC_MPI_MASTER_NODE) mechanic_welcome();

#ifdef WE_ARE_ON_DARWIN
  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "We are running on DARWIN platform\n");
  }
#endif

#ifdef WE_ARE_ON_LINUX
  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "We are running on LINUX platform\n");
  }
#endif

  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "MPI is ready on node %s\n", processor_name);
  }

  /* HDF5 INIT */
  mstat = H5open();
  if (mstat < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "HDF5 OPEN failed on node %d\n", node);
    mechanic_abort(MECHANIC_ERR_HDF);
    exit(EXIT_FAILURE);
  }

  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "HDF is ready\n");
  }

  /* CONFIGURATION START */

  /* Assign LRC defaults */
  head = LRC_assignDefaults(cs);
  if (head == NULL) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "LRC failed on node %d\n", node);
    mechanic_abort(MECHANIC_ERR_SETUP);
    exit(EXIT_FAILURE);
  }

  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "LRC is ready\n");
  }
  
  /* Assign POPT defaults */
  name = MECHANIC_NAME_DEFAULT;
  module_name = MECHANIC_MODULE_DEFAULT;
  ConfigFile = MECHANIC_CONFIG_FILE_DEFAULT;
  ModuleConfigFile = MECHANIC_CONFIG_FILE_DEFAULT;
  xres = atoi(MECHANIC_XRES_DEFAULT);
  yres = atoi(MECHANIC_YRES_DEFAULT);
  checkpoint = atoi(MECHANIC_CHECKPOINT_DEFAULT);
  mode = atoi(MECHANIC_MODE_DEFAULT);

  /* COMMAND LINE PARSING
   * All nodes read commandline options, because of problems of finishing MPI
   * in case of help/unknown option */
  poptcon = poptGetContext(NULL, argc, (const char **) argv, cmdopts, 0);
  optvalue = poptGetNextOpt(poptcon);

  /* This is specific for command line arguments handling 
   * We need to check if name, module, checkpoint file are not 
   * cli options 
   *
   * FOR FUTURE ME:
   * I couldn't do it better with popt or getopt_long,
   * so I need to check it by self.
   *
   * Btw. How to check if the specific arg is provided?
   * */
  if ((name != NULL) && (validate_arg(name) < 0)) goto setupfinalize;
  if ((module_name != NULL) && (validate_arg(module_name) < 0)) goto setupfinalize;
  if ((ConfigFile != NULL) && (validate_arg(ConfigFile) < 0)) goto setupfinalize;
  if ((ModuleConfigFile != NULL) && (validate_arg(ModuleConfigFile) < 0)) goto setupfinalize;
  if ((CheckpointFile != NULL) && (validate_arg(CheckpointFile) < 0)) goto setupfinalize;

  /* POPT error detection */
  if (optvalue < -1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      mechanic_message(MECHANIC_MESSAGE_WARN, "%s: %s\n",
        poptBadOption(poptcon, POPT_BADOPTION_NOALIAS),
        poptStrerror(optvalue));
      poptPrintHelp(poptcon, stdout, poptflags);
    }
    goto setupfinalize;
  }

  /* Long help message set */
  if (help == 1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      poptPrintHelp(poptcon, stdout, 0);
    }
    goto setupfinalize;
  }

  /* Brief help message set */
  if (usage == 1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      poptPrintUsage(poptcon, stdout, 0);
    }
    goto setupfinalize;
  }

  /* Restartmode 
   *
   * NOTE:
   * Restartmode is handled according to the present of the CheckpointFile.
   * By default, CheckpointFile is NULL, you can override it by specifing
   * -r file or --restart=file in the commandline.
   */
  if (CheckpointFile) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      mechanic_message(MECHANIC_MESSAGE_INFO,
        "Checkpoint file specified: %s\n", CheckpointFile);
      /* STEP 1: Check the health of the checkpoint file */
      if (stat(CheckpointFile, &ct) == 0) {

        /* The checkpoint file should be valid HDF5 file */
        if (H5Fis_hdf5(CheckpointFile) > 0) {

          /* Check if the file is a valid Mechanic file */
          mechanic_validate_file(CheckpointFile);

          /* At this point we can try restart the computations */
          mechanic_message(MECHANIC_MESSAGE_CONT,
            "Restartmode, trying to checkpoint file %s\n", CheckpointFile);
          restartmode = 1;
        } else {
          mechanic_message(MECHANIC_MESSAGE_ERR,
            "Specified checkpoint file %s is not a valid Mechanic file\n", CheckpointFile);
          goto checkpointabort;
        }
      } else {
        mechanic_message(MECHANIC_MESSAGE_ERR,
          "Specified checkpoint file %s is not valid\n", CheckpointFile);
        goto checkpointabort;
      }
    }
  }

  /* PROCESS CONFIGURATION DATA */
  if (node == MECHANIC_MPI_MASTER_NODE) {

    /* Config file is set and we are not in restart mode */
    if (strcmp(ConfigFile, MECHANIC_CONFIG_FILE_DEFAULT) != 0
       && restartmode == 0) {
      useConfigFile = 1;
    }
    /* Module config file is set and we are not in restart mode */
    if (strcmp(ModuleConfigFile, MECHANIC_CONFIG_FILE_DEFAULT) != 0
       && restartmode == 0) {
      useModuleConfigFile = 1;
    }

    /* STEP 1A: Read config file, if any.
     * This will override LRC_configDefaults
     * In restart mode we try provided checkpoint file */
    if (restartmode == 1) {
      allopts = readCheckpointConfig(CheckpointFile, MECHANIC_CONFIG_GROUP, head);
    } else {
      allopts = readDefaultConfig(ConfigFile, useConfigFile, head);
    }

    /* STEP 1B: Reset POPT defaults
     * We need to reassign popt defaults in case of config file, if any.
     * If there was no config file, defaults will be untouched.
     *
     * Commandline is completely ignored in restartmode */
    if (restartmode == 0) {
      name = LRC_getOptionValue("default", "name", head);
      module_name = LRC_getOptionValue("default", "module", head);
      ModuleConfigFile = LRC_getOptionValue("default", "mconfig", head);
      mode = LRC_option2int("default", "mode", head);
      xres = LRC_option2int("default", "xres", head);
      yres = LRC_option2int("default", "yres", head);
      checkpoint = LRC_option2int("logs", "checkpoint", head);

      /* STEP 2A: Read commandline options, if any.
       * This will override popt defaults. */
      poptResetContext(poptcon);
      poptcon = poptGetContext(NULL, argc, (const char **) argv, cmdopts, 0);
      optvalue = poptGetNextOpt(poptcon);

      /* STEP 2B: Modify options by commandline
       * If there was no commandline options, LRC table will be untouched. */
      LRC_modifyOption("default", "name", name, LRC_STRING, head);
      LRC_modifyOption("default", "module", module_name, LRC_STRING, head);
      LRC_modifyOption("default", "mconfig", ModuleConfigFile, LRC_STRING, head);

      LRC_itoa(convstr, xres, LRC_INT);
      LRC_modifyOption("default", "xres", convstr, LRC_INT, head);

      LRC_itoa(convstr, yres, LRC_INT);
      LRC_modifyOption("default", "yres", convstr, LRC_INT, head);

      LRC_itoa(convstr, mode, LRC_INT);
      LRC_modifyOption("default", "mode", convstr, LRC_INT, head);

      LRC_itoa(convstr, checkpoint, LRC_INT);
      LRC_modifyOption("logs", "checkpoint", convstr, LRC_INT, head);

    }

    /* We allow to override checkpoint file interval in the restartmode */
    if (restartmode == 1) {
      checkpoint = LRC_option2int("logs", "checkpoint", head);
      poptResetContext(poptcon);
      poptcon = poptGetContext(NULL, argc, (const char **) argv, cmdopts, 0);
      optvalue = poptGetNextOpt(poptcon);
      LRC_itoa(convstr, checkpoint, LRC_INT);
      LRC_modifyOption("logs", "checkpoint", convstr, LRC_INT, head);
    }

    /* STEP 3: Options are processed, we can now assign config values. */
    mstat = assignConfigValues(&cd, head);
    mechanic_check_mstat(mstat);

    mechanic_message(MECHANIC_MESSAGE_DEBUG,"Config file contents:\n\n");
    mechanic_printConfig(&cd, MECHANIC_MESSAGE_DEBUG);

    /* Security check: if mpi_size = 1 switch to masteralone mode */
    if (mpi_size == 1) {
      cd.mode = 0;
      mechanic_message(MECHANIC_MESSAGE_WARN,
        "MPI COMM SIZE = 1. Will switch to master alone mode now\n");
    }

    /* Security check: resolution */
    if (cd.xres == 0 || cd.yres == 0) {
       mechanic_message(MECHANIC_MESSAGE_ERR,
         "X/Y task pool resolution should not be set to 0!\n");

       mechanic_message(MECHANIC_MESSAGE_ERR,
         "If You want to do only one simulation, please set xres = 1, yres = 1\n");

       mechanic_error(MECHANIC_ERR_SETUP);
    }

    mechanic_message(MECHANIC_MESSAGE_INFO,
      "Mechanic will use these startup values:\n\n");
    mechanic_printConfig(&cd, MECHANIC_MESSAGE_CONT);

    /* Backup master data file, if exists */
    if (stat(cd.datafile,&st) == 0) {

      opreflen = strlen(MECHANIC_FILE_OLD_PREFIX);
      dlen = strlen(cd.datafile);
      olen = dlen + opreflen + 2*sizeof(char*);

      /* Allocate memory for the backup name */
      oldfile = calloc(olen + sizeof(char*), sizeof(char*));
      if (oldfile == NULL) mechanic_error(MECHANIC_ERR_MEM);

      strncpy(oldfile, MECHANIC_FILE_OLD_PREFIX, opreflen);
      oldfile[opreflen] = LRC_NULL;

      strncat(oldfile, cd.datafile, dlen);
      oldfile[olen] = LRC_NULL;

      mechanic_message(MECHANIC_MESSAGE_WARN,
        "File %s exists!\n", cd.datafile);
      mechanic_message(MECHANIC_MESSAGE_WARN,
        "I will back it up for You now\n");
      mechanic_message(MECHANIC_MESSAGE_WARN,
        "Backuped file: %s\n",oldfile);

      /* Now we can safely rename files */
      if (restartmode == 1) {
        mstat = mechanic_copy(cd.datafile, oldfile);
        mechanic_check_mstat(mstat);
        /* At this point we have a backup of master file. In restart mode we
         * have to start the simulation from the point of the provided
         * checkpoint file. To save number of additional work, we simply make
         * our checkpoint file new master file.*/
        mstat = mechanic_copy(CheckpointFile, cd.datafile);
        mechanic_check_mstat(mstat);
      } else {
        rename(cd.datafile,oldfile);
      }

      free(oldfile);
    }
  }
 /* CONFIGURATION END */

 /* MPI CONFIGURATION BCAST
  * Inform workers what it is all about. */
 if (mpi_size > 1) {

   /* Send to workers information about lengths of important strings */
    lengths[0] = 0; lengths[1] = 0; lengths[2] = 0; lengths[3] = 0;

    if (node == MECHANIC_MPI_MASTER_NODE) {

      lengths[0] = (int) strlen(cd.name);
      lengths[1] = (int) strlen(cd.datafile);
      lengths[2] = (int) strlen(cd.module);
      lengths[3] = (int) strlen(cd.mconfig);

    }
      
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node[%d] ready\n", node);
      
    MPI_Bcast(lengths, 4, MPI_INT, MECHANIC_MPI_DEST, MPI_COMM_WORLD);

    cd.name_len = lengths[0];
    cd.datafile_len = lengths[1];
    cd.module_len = lengths[2];
    cd.mconfig_len = lengths[3];

    mstat = buildConfigDataType(lengths, cd, &ConfigType);
    if (mstat < 0) mechanic_message(MECHANIC_MESSAGE_ERR, "ConfigDataType committing failed.\n");
    MPI_Bcast(&cd, 1, ConfigType, MECHANIC_MPI_DEST, MPI_COMM_WORLD);
    MPI_Type_free(&ConfigType);

    cd.name[lengths[0]] = LRC_NULL;
    cd.datafile[lengths[1]] = LRC_NULL;
    cd.module[lengths[2]] = LRC_NULL;
    cd.mconfig[lengths[3]] = LRC_NULL;

    mechanic_message(MECHANIC_MESSAGE_DEBUG,
      "Node[%d] lengths[%d, %d, %d, %d]\n",
      node, cd.name_len, cd.datafile_len, cd.module_len, cd.mconfig_len);

    mechanic_message(MECHANIC_MESSAGE_DEBUG,
      "Node [%d] received following configuration:\n\n", node);
    mechanic_printConfig(&cd, MECHANIC_MESSAGE_DEBUG);

    /* If we are in masteralone mode, finalize workers */
    if (node != MECHANIC_MPI_MASTER_NODE && cd.mode == MECHANIC_MODE_MASTERALONE) 
      goto setupfinalize;
  }

  /* Assign some fair defaults */
  md.mrl = MECHANIC_MRL_DEFAULT;
  md.irl = MECHANIC_IRL_DEFAULT;
  md.api = MECHANIC_MODULE_API;
  md.schemasize = 1;
  md.options = 0;
  md.mconfig = NULL;

  /* Initialize internals and load modules  */
  internals = mechanic_internals_init(mpi_size, node, &md, &cd);

  /* Module init */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module init\n");
  init = mechanic_load_sym(&internals, "init", MECHANIC_MODULE_ERROR, MECHANIC_TEMPLATE);
  if (init) mstat = init(internals.mpi_size, internals.node, internals.info, internals.config);
  mechanic_check_mstat(mstat);
  
  /* This will override defaults */
  internals = mechanic_internals_init(mpi_size, node, &md, &cd);

  /* ICE file name */
  prepare_ice(&internals);

  /* ICE file presence check */
  if ((node == MECHANIC_MPI_MASTER_NODE) && (mechanic_ice(&internals) == MECHANIC_ICE)) {
      mechanic_message(MECHANIC_MESSAGE_WARN, "ICE file is present, will abort now\n");
      mechanic_abort(MECHANIC_ICE);
  }

  /* Initialize schema (module storage and setup) */
  mechanic_internals_schema_init(node, &md, &internals);
  
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "info.mrl = %d\n", internals.info->mrl);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "info.irl = %d\n", internals.info->irl);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "info.api = %d\n", internals.info->api);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "info.options = %d\n", internals.info->options);

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "mrl = %d\n", md.mrl);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "irl = %d\n", md.irl);

  if (internals.info->mrl <= 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Master result length must be greater than 0\n");
    mechanic_error(MECHANIC_ERR_SETUP);
  }

  if (internals.info->irl <= 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Initial condition length must be greater than 0\n");
    mechanic_error(MECHANIC_ERR_SETUP);
  }
  
  /* Module setup schema */
  if (internals.info->options > 0) {
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module setup schema\n");
    query = mechanic_load_sym(&internals, "setup_schema", MECHANIC_MODULE_SILENT, MECHANIC_NO_TEMPLATE);
    if (query) mstat = query(internals.info);
    mechanic_check_mstat(mstat);

    /* LRC module configuration */
    module_head = LRC_assignDefaults(internals.info->mconfig);
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node [%d] options = %d\n", node, internals.info->options);
    ccc = allocateLRCMPIStruct(internals.info->options);

    /* Read module setup file on the master node only */
    if (node == MECHANIC_MPI_MASTER_NODE && restartmode == 0) {
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "UseModuleConfigFile = %d\n", useModuleConfigFile);
      readDefaultConfig(ModuleConfigFile, useModuleConfigFile, module_head);
    }
    if (node == MECHANIC_MPI_MASTER_NODE && restartmode == 1) {
      readCheckpointConfig(CheckpointFile, internals.config->module, module_head);
    }
    
    /* 
     * Important: put something into LRC-MPI structure. It will be defaults on worker
     * nodes and eventually overrides on the master.
     */
    LRC2MPI(ccc, module_head);
    
    /* Broadcast LRC module configuration */
    mstat = LRC_datatype(ccc[0], &lrc_mpi_t);
    if (mstat < 0) mechanic_message(MECHANIC_MESSAGE_ERR, "LRC_Datatype committing failed.\n");

    /*
     * For some reason BCAST crashes here, in Fortran such thing works, here,
     * unfortunately no.
     */
 //   MPI_Bcast(&ccc, internals.info->options, lrc_mpi_t, MECHANIC_MPI_DEST, MPI_COMM_WORLD);

    for (i = 0; i < internals.info->options; i++) {
      if (node == MECHANIC_MPI_MASTER_NODE) {
        for (n = 1; n < mpi_size; n++) {
          MPI_Send(&ccc[i], 1, lrc_mpi_t, n, MECHANIC_MPI_STANDBY_TAG, MPI_COMM_WORLD);
        }
      } else {
        MPI_Recv(&ccc[i], 1, lrc_mpi_t, 0, MECHANIC_MPI_STANDBY_TAG, MPI_COMM_WORLD, &mpi_status);
      }
    }

    MPI_Type_free(&lrc_mpi_t);

    /* Modify module configuration */
    for (i = 0; i < internals.info->options; i++) {
      LRC_modifyOption(ccc[i].space, ccc[i].name, ccc[i].value, ccc[i].type, module_head);
    }

    /* Assign current configuration so that it could be passed to API */
    internals.info->moptions = module_head;

    free(ccc);
  }

  /* There are some special data in the module,
   * thus we have to create master data file after module has been
   * successfully loaded.
   */
  if (node == MECHANIC_MPI_MASTER_NODE && restartmode == 0) {

    /* Create master datafile */
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Create master data file\n");
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Filename is %s\n", cd.datafile);
    file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  
    /* Storage schema */
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module storage schema\n");
    schema = mechanic_load_sym(&internals, "storage_schema", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
    if (schema) mstat = schema(internals.mpi_size, internals.node, internals.info, internals.config);
    mechanic_check_mstat(mstat);
    
    mstat = H5createMasterDataScheme(file_id, &md, &cd);
    mechanic_check_mstat(mstat);

    /* First of all, save configuration */
    LRC_HDF5Writer(file_id, MECHANIC_CONFIG_GROUP, head);
    
    /* Module configuration if any */
    if (internals.info->options > 0) {
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "Config group: %s\n", internals.config->module);
      LRC_HDF5Writer(file_id, internals.config->module, module_head);
    }

    H5Fclose(file_id);
  }

  /* At this point all nodes know everything about the run, and
   * the master file has been prepared, configuration stored. We can call
   * additional query on all nodes here, if needed. */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module comm_prepare\n");
  query = mechanic_load_sym(&internals, "comm_prepare", MECHANIC_MODULE_SILENT, MECHANIC_NO_TEMPLATE);
  if (query) mstat = query(internals.mpi_size, internals.node, internals.info, internals.config);
  mechanic_check_mstat(mstat);

  /* Bootstrap done, call modes */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Loading mode\n");
  switch (cd.mode) {
    case MECHANIC_MODE_MASTERALONE:
      mstat = mechanic_mode_masteralone(&internals);
      mechanic_check_mstat(mstat);
      break;
    case MECHANIC_MODE_FARM:
      mstat = mechanic_mode_farm(&internals);
      mechanic_check_mstat(mstat);
      break;
    default:
      break;
  }
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Mode finished\n");

  /* To be sure all nodes reach here at the same time */
  if (cd.mode != MECHANIC_MODE_MASTERALONE) {
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "MPI Barrier reached\n");
    MPI_Barrier(MPI_COMM_WORLD);
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "MPI Barrier crossed\n");
  }

  /* Module cleanup */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module cleanup\n");
  cleanup = mechanic_load_sym(&internals, "cleanup", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
  if (cleanup) mstat = cleanup(internals.mpi_size, internals.node, internals.info, internals.config);
  mechanic_check_mstat(mstat);

  /* Mechanic cleanup */
  mechanic_internals_close(&internals);
  mechanic_message(MECHANIC_MESSAGE_DEBUG,"Node[%d] Internals closed.\n", node);

setupfinalize:

  /* HDF5 finalize */
  H5close();
  mechanic_message(MECHANIC_MESSAGE_DEBUG,"Node[%d] HDF5 closed.\n", node);

  /* Free POPT */
  poptFreeContext(poptcon);
  mechanic_message(MECHANIC_MESSAGE_DEBUG,"Node[%d] POPT closed.\n", node);

  /* Cleanup LRC */
  LRC_cleanup(head);
  if (internals.info->options > 0) LRC_cleanup(module_head);
  mechanic_message(MECHANIC_MESSAGE_DEBUG,"Node[%d] LRC closed.\n", node);

  /* Finalize */
  mechanic_finalize(node);
  mechanic_message(MECHANIC_MESSAGE_DEBUG,"Node[%d] MPI closed.\n", node);

  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "Mechanic finished his job\n");
    mechanic_message(MECHANIC_MESSAGE_CONT, "Have a nice day!\n\n");
  }

  exit(EXIT_SUCCESS);

checkpointabort:
  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_abort(MECHANIC_ERR_CHECKPOINT);
  }
}

