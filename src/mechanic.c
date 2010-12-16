/*
 * MECHANIC
 * Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
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
 * http://mechanics.astri.umk.pl/projects/mechanic
 *
 * User guide should be provided with the package or
 * http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
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
 * @date 2010
 *
 * @todo
 *   - HDF5 error handling with H5E (including file storage)
 *   - multifarm mode
 *   - engines
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
 * http://git.astri.umk.pl. The @c Experimental branch containes
 * all bleeding-edge stuff.
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
 * @M uses @c Waf build system, see http://code.google.com/p/waf for detailes.
 * @c Waf is build in Python, you should have at least Python 2.3 installed
 * on your system.
 *
 * To download the latest snapshot of @M try
 * @code
 * http://git.astri.umk.pl/project/mechanic
 * @endcode
 *
 * We try to keep as less requirements as possible to use @M. To compile our
 * software you need at least:
 *
 * - @c MPI2 implementation (we prefer @c OpenMPI, and @M was tested with it)
 * - @c HDF5, at least 1.8
 * - @c LibReadConfig with @c HDF5 support -- @c LRC can be downloaded from
 *   our git repository, since it is a helper tool builded especially for @M,
 *   but can be used independly. You need to compile it with @c --enable-hdf
 *   flag
 * - @c Popt library (should be already installed on your system)
 * - C compiler (@c gcc 4.3 should do the job)
 *
 * If you are going to use Fortran code, you need to install/compile proper 
 * @c gcc and @c MPI2 extensions.
 *
 * Compilation is similar to standard @c Autotools path:
 *
 * @code
 * ./waf configure
 * ./waf build
 * ./waf install
 * @endcode
 *
 * The default installation path is set to @c /usr/local, but you can change
 * it with @c --prefix flag.
 *
 * By default, @M comes only with core. However, you can consider building
 * additional modules, engines and libraries, as follows:
 * @code
 * --with-modules=list,of,modules
 * --with-engines=list,of,engines
 * --with-libs=list,of,libs
 * @endcode
 *
 * Available modules:
 * - @c hello, see @ref hello
 * - @c echo, see @ref echo
 * - @c mandelbrot, see @ref mandelbrot
 *
 * Available engines (currently only templates):
 * - @c odex
 * - @c taylor
 * - @c gpu
 *
 * Available libs:
 * - @c orbit -- a library for handling common tasks of celestial mechanics,
 *   i.e orbital elements conversion, see @ref orbit
 *
 * To build Fortran 2003 bindings, use @c --with-fortran option. There are also
 * F2003 sample modules available (@c --with-fortran-modules):
 * - @c fhello
 * - @c map
 *
 * You will find detailed instruction of using Fortran bindings in
 * @ref f2003bind.
 *
 * The documentation can will be builded, with @c --with-doc option.
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
 * @idoc core/mechanic_core_tools.c DEVEL
 *
 * @page troubleshooting Troubleshooting
 * @idoc core/mechanic_core_errors.c ERRORS
 *
 */

int main(int argc, char* argv[]){

  /* POPT Helpers */
  char* module_name;
  char* name;
  int mode = 0;
  /*unsigned long*/ int xres = 0;
  /*unsigned long*/ int yres = 0;
  /*unsigned long*/ int checkpoint = 0;
  int poptflags = 0;
  int useConfigFile = 0;
  char optvalue;
  int restartmode = 0;
  char convstr[MECHANIC_MAXLENGTH];
  poptContext poptcon;
  help = 0;
  usage = 0;

  module_handler modhand;

  char* module_file;
  char* oldfile;
  size_t olen, opreflen, dlen;

  /* HDF Helpers */
  hid_t file_id;

  /* MECHANIC Helpers */
  configData cd; /* struct for command line args */
  moduleInfo md; /* struct for module info */
  int mstat; /* mechanic internal error value */
  struct stat st; /* stat.h */
  struct stat ct; /* stat.h */

  /* MPI Helpers */
  int mpi_rank;
  int mpi_size;
  int node = 0;
  MPI_Status mpi_status;
  int lengths[4];
  int i = 0;
  char pack_buffer[MECHANIC_MAXLENGTH];
  int pack_position;

  /* LRC defaults */
  LRC_configDefaults cs[] = {
    {"default", "name", MECHANIC_NAME_DEFAULT, LRC_STRING},
    {"default", "xres", MECHANIC_XRES_DEFAULT, LRC_INT},
    {"default", "yres", MECHANIC_YRES_DEFAULT, LRC_INT},
    {"default", "module", MECHANIC_MODULE_DEFAULT, LRC_STRING},
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
    /*{"multifarm", MECHANIC_MODE_MULTIFARM_P, POPT_ARG_VAL, &mode,
      MECHANIC_MODE_MULTIFARM, "MPI multi task farm", NULL},*/
#ifdef HAVE_CUDA_H
    {"cuda", MECHANIC_MODE_CUDA_P, POPT_ARG_VAL, &mode,
      MECHANIC_MODE_CUDA, "CUDA based farm", NULL},
#endif
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

  /* MPI INIT */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  node = mpi_rank;

#ifdef WE_ARE_ON_DARWIN
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "We are running on DARWIN platform\n");
#endif

#ifdef WE_ARE_ON_LINUX
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "We are running on LINUX platform\n");
#endif

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "MPI started\n");

  /* HDF5 INIT */
  H5open();

  /* CONFIGURATION START */
  if (node == MECHANIC_MPI_MASTER_NODE) {

    /* Assign LRC defaults */
    LRC_assignDefaults(cs);
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "LRC defaults assigned\n");

    /* Assign POPT defaults */
    name = MECHANIC_NAME_DEFAULT;
    module_name = MECHANIC_MODULE_DEFAULT;
    ConfigFile = MECHANIC_CONFIG_FILE_DEFAULT;
    xres = atoi(MECHANIC_XRES_DEFAULT);
    yres = atoi(MECHANIC_YRES_DEFAULT);
    checkpoint = atoi(MECHANIC_CHECKPOINT_DEFAULT);
    mode = atoi(MECHANIC_MODE_DEFAULT);

  }

  /* COMMAND LINE PARSING
   * All nodes read commandline options, because of problems of finishing MPI
   * in case of help/unknown option */
  poptcon = poptGetContext(NULL, argc, (const char **) argv, cmdopts, 0);
  optvalue = poptGetNextOpt(poptcon);

  /* Bad option handling */
  if (optvalue < -1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      mechanic_message(MECHANIC_MESSAGE_WARN, "%s: %s\n",
        poptBadOption(poptcon, POPT_BADOPTION_NOALIAS),
        poptStrerror(optvalue));
      poptPrintHelp(poptcon, stdout, poptflags);
    }
    poptFreeContext(poptcon);
    mechanic_finalize(node);
    return 0;
  }

  /* Long help message set */
  if (help == 1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      poptPrintHelp(poptcon, stdout, 0);
    }
    poptFreeContext(poptcon);
    mechanic_finalize(node);
    return 0;
  }

  /* Brief help message set */
  if (usage == 1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      poptPrintUsage(poptcon, stdout, 0);
    }
    poptFreeContext(poptcon);
    mechanic_finalize(node);
    return 0;
  }

  if (node == MECHANIC_MPI_MASTER_NODE) mechanic_welcome();

  /* Restartmode */
  if (CheckpointFile) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      mechanic_message(MECHANIC_MESSAGE_DEBUG,
          "Checkpoint file specified: %s\n", CheckpointFile);
      /* STEP 1: Check the health of the checkpoint file */
      if (stat(CheckpointFile, &ct) == 0) {

        /* The checkpoint file should be valid HDF5 file */
        if (H5Fis_hdf5(CheckpointFile) > 0) {

          /* Check if the file is a valid Mechanic file */
          // mechanic_validate_file(CheckpointFile)

          /* We can say, we can try restart the computations */
          mechanic_message(MECHANIC_MESSAGE_INFO,
              "We are in restart mode\n");
          mechanic_message(MECHANIC_MESSAGE_CONT,
              "Mechanic will ignore command line options\n");
          mechanic_message(MECHANIC_MESSAGE_CONT,
              "and use configuration stored in %s\n", CheckpointFile);

          restartmode = 1;
        } else {
          mechanic_message(MECHANIC_MESSAGE_ERR,
              "We are in restart mode,\n");
          mechanic_message(MECHANIC_MESSAGE_CONT,
              "but specified checkpoint file %s\n", CheckpointFile);
          mechanic_message(MECHANIC_MESSAGE_CONT,
              "is not valid Mechanic file\n");
          mechanic_abort(MECHANIC_ERR_CHECKPOINT);
        }
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

    /* STEP 1A: Read config file, if any.
     * This will override LRC_configDefaults
     * In restart mode we try provided checkpoint file */
    if (restartmode == 1) {
      allopts = readCheckpointConfig(CheckpointFile);
    } else {
      allopts = readDefaultConfig(ConfigFile, useConfigFile);
    }

    /* STEP 1B: Reset POPT defaults
     * We need to reassign popt defaults in case of config file, if any.
     * If there was no config file, defaults will be untouched.
     *
     * Commandline is completely ignored in restartmode */
    if (restartmode == 0) {
      name = LRC_getOptionValue("default", "name");
      module_name = LRC_getOptionValue("default", "module");
      mode = LRC_option2int("default", "mode");
      xres = LRC_option2int("default", "xres");
      yres = LRC_option2int("default", "yres");
      checkpoint = LRC_option2int("logs", "checkpoint");

      /* STEP 2A: Read commandline options, if any.
       * This will override popt defaults. */
      poptResetContext(poptcon);
      poptcon = poptGetContext(NULL, argc, (const char **) argv, cmdopts, 0);
      optvalue = poptGetNextOpt(poptcon);

      /* STEP 2B: Modify options by commandline
       * If there was no commandline options, LRC table will be untouched. */
      LRC_modifyOption("default", "name", name, LRC_STRING);
      LRC_modifyOption("default", "module", module_name, LRC_STRING);

      LRC_itoa(convstr, xres, LRC_INT);
      LRC_modifyOption("default", "xres", convstr, LRC_INT);

      LRC_itoa(convstr, yres, LRC_INT);
      LRC_modifyOption("default", "yres", convstr, LRC_INT);

      LRC_itoa(convstr, mode, LRC_INT);
      LRC_modifyOption("default", "mode", convstr, LRC_INT);

      LRC_itoa(convstr, checkpoint, LRC_INT);
      LRC_modifyOption("logs", "checkpoint", convstr, LRC_INT);

    }

    /* We allow to override checkpoint file interval in the restartmode */
    if (restartmode == 1) {
      checkpoint = LRC_option2int("logs", "checkpoint");
      poptResetContext(poptcon);
      poptcon = poptGetContext(NULL, argc, (const char **) argv, cmdopts, 0);
      optvalue = poptGetNextOpt(poptcon);
      LRC_itoa(convstr, checkpoint, LRC_INT);
      LRC_modifyOption("logs", "checkpoint", convstr, LRC_INT);
    }

    /* STEP 3: Options are processed, we can now assign config values. */
    mstat = assignConfigValues(&cd);

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
         "X/Y map resolution should not be set to 0!\n");

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
        mechanic_copy(cd.datafile, oldfile);
        /* At this point we have a backup of master file. In restart mode we
         * have to start the simulation from the point of the provided
         * checkpoint file. To save number of additional work, we simply make
         * our checkpoint file new master file.*/
        mechanic_copy(CheckpointFile, cd.datafile);
      } else {
        rename(cd.datafile,oldfile);
      }

      free(oldfile);
    }
  }
 /* CONFIGURATION END */

  /* Send standby message to slaves
   * This is because slaves don't know anything about the run until master
   * node send the configuration with bcast. Without this part, code will
   * raise some segfaults and endless-loops. The standby message containes
   * the mode in which we are working.
   */
  if (mpi_size > 1) {
    if (node == MECHANIC_MPI_MASTER_NODE) {
      for (i = 1; i < mpi_size; i++) {
        if (cd.mode == MECHANIC_MODE_MASTERALONE) {
          mechanic_message(MECHANIC_MESSAGE_WARN,
            "Terminating SLAVE[%d]\n", i);

          MPI_Send(&cd.mode, 1, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG,
            MPI_COMM_WORLD);
        } else {
          MPI_Send(&cd.mode, 1, MPI_INT, i, MECHANIC_MPI_STANDBY_TAG,
            MPI_COMM_WORLD);
        }
      }
    } else {
      MPI_Recv(&cd.mode, 1, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);
      if (mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) {
        mechanic_finalize(node);
        return 0;
      }
    }
  }

 /* MPI CONFIGURATION BCAST
  * Inform slaves what it is all about. */
   if (cd.mode != MECHANIC_MODE_MASTERALONE) {

     /* Send to slaves information about lengths of important strings */
      if (node == MECHANIC_MPI_MASTER_NODE) {

        lengths[0] = (int) strlen(cd.name);
        lengths[1] = (int) strlen(cd.datafile);
        lengths[2] = (int) strlen(cd.module);

        cd.name_len = lengths[0];
        cd.datafile_len = lengths[1];
        cd.module_len = lengths[2];

        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Node[%d] lengths[%d, %d, %d]\n",
            node, lengths[0], lengths[1], lengths[2]);

        for (i = 1; i < mpi_size; i++) {
          MPI_Send(&lengths, 3, MPI_INT, i, MECHANIC_MPI_STANDBY_TAG,
            MPI_COMM_WORLD);
        }

      } else {

        mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node[%d] ready\n", node);

        /* Slave node doesn't read config file, so we need to receive first
         * information about lengths of string data */

        MPI_Recv(&lengths, 3, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG,
          MPI_COMM_WORLD, &mpi_status);
        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Node[%d] lengths[%d, %d, %d]\n",
            node, lengths[0], lengths[1], lengths[2]);

        cd.name_len = lengths[0];
        cd.datafile_len = lengths[1];
        cd.module_len = lengths[2];
      }

      if (node == MECHANIC_MPI_MASTER_NODE) {

        pack_position = 0;

        MPI_Pack(cd.name, lengths[0], MPI_CHAR, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(cd.datafile, lengths[1], MPI_CHAR, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(cd.module, lengths[2], MPI_CHAR, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(&cd.xres, 1, MPI_INT, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(&cd.yres, 1, MPI_INT, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(&cd.checkpoint, 1, MPI_INT, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(&cd.restartmode, 1, MPI_INT, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Pack(&cd.mode, 1, MPI_INT, pack_buffer,
          MECHANIC_MAXLENGTH, &pack_position, MPI_COMM_WORLD);

        MPI_Bcast(pack_buffer, MECHANIC_MAXLENGTH, MPI_PACKED,
          MECHANIC_MPI_DEST, MPI_COMM_WORLD);

      } else {

        /* Receive data from master node */
        pack_position = 0;
        MPI_Bcast(pack_buffer, MECHANIC_MAXLENGTH, MPI_PACKED,
          MECHANIC_MPI_DEST, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          cd.name, lengths[0], MPI_CHAR, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          cd.datafile, lengths[1], MPI_CHAR, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          cd.module, lengths[2], MPI_CHAR, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          &cd.xres, 1, MPI_INT, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          &cd.yres, 1, MPI_INT, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          &cd.checkpoint, 1, MPI_INT, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          &cd.restartmode, 1, MPI_INT, MPI_COMM_WORLD);

        MPI_Unpack(pack_buffer, MECHANIC_MAXLENGTH, &pack_position,
          &cd.mode, 1, MPI_INT, MPI_COMM_WORLD);

        cd.name[lengths[0]] = LRC_NULL;
        cd.datafile[lengths[1]] = LRC_NULL;
        cd.module[lengths[2]] = LRC_NULL;

        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Node[%d] lengths[%d, %d, %d]\n",
            node, cd.name_len, cd.datafile_len, cd.module_len);

        mechanic_message(MECHANIC_MESSAGE_DEBUG,
          "Node [%d] received following configuration:\n\n", node);
        mechanic_printConfig(&cd, MECHANIC_MESSAGE_DEBUG);

      }

   }

  /* Create module file name */
  module_file = mechanic_module_filename(cd.module);

  /* Assign some fair defaults */
  md.mrl = MECHANIC_MRL_DEFAULT;
  md.irl = MECHANIC_IRL_DEFAULT;
  md.api = MECHANIC_MODULE_API;

  /* Load module */
  modhand = mechanic_module_open(module_file);

  /* Module init */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module init\n");
  if (node == MECHANIC_MPI_MASTER_NODE) {
    init = mechanic_load_sym(modhand,cd.module, "init", "master_init",
        MECHANIC_MODULE_ERROR);
  } else if ((cd.mode != MECHANIC_MODE_MASTERALONE) && (node != MECHANIC_MPI_MASTER_NODE)) {
    init = mechanic_load_sym(modhand,cd.module, "init", "slave_init",
        MECHANIC_MODULE_ERROR);
  } else {
    init = mechanic_load_sym(modhand,cd.module, "init", "init",
        MECHANIC_MODULE_ERROR);
  }
  if (init) mstat = init(mpi_size, node, &md, &cd);
  mechanic_check_mstat(mstat);

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "mrl = %d\n", md.mrl);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "irl = %d\n", md.irl);

  if (md.mrl <= 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Master result length must be greater than 0\n");
    mechanic_error(MECHANIC_ERR_SETUP);
  }

  if (md.irl <= 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Initial condition length must be greater than 0\n");
    mechanic_error(MECHANIC_ERR_SETUP);
  }

  /* Module query */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module query\n");
  query = mechanic_load_sym(modhand,cd.module, "query", "query", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(mpi_size, node, &md, &cd);
  mechanic_check_mstat(mstat);

  /* There are some special data in the module,
   * thus we have to create master data file after module has been
   * successfully loaded.
   */
  if (node == MECHANIC_MPI_MASTER_NODE && restartmode == 0) {
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Create master data file\n");
    /* Create master datafile */
    file_id = H5Fcreate(cd.datafile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    mstat = H5createMasterDataScheme(file_id, &md, &cd);
    mechanic_check_mstat(mstat);

    /* First of all, save configuration */
    LRC_HDF5Writer(file_id);
    H5Fclose(file_id);
  }

  /* Now load proper routines */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Loading mode\n");
  switch (cd.mode) {
    case MECHANIC_MODE_MASTERALONE:
      mstat = mechanic_mode_masteralone(mpi_size, node, modhand, &md, &cd);
      mechanic_check_mstat(mstat);
      break;
    case MECHANIC_MODE_FARM:
      mstat = mechanic_mode_farm(mpi_size, node, modhand, &md, &cd);
      mechanic_check_mstat(mstat);
      break;
#ifdef HAVE_CUDA_H
    case MECHANIC_MODE_CUDA:
      mstat = mechanic_mode_cuda(mpi_size, node, modhand, &md, &cd);
      mechanic_check_mstat(mstat);
      break;
#endif
    /*case MECHANIC_MODE_MULTIFARM:
      mstat = mechanic_mode_multifarm(mpi_size, node, modhand, &md, &cd);
      break;*/
    default:
      break;
  }

  /* Module cleanup */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Calling module cleanup\n");
  if (node == MECHANIC_MPI_MASTER_NODE) {
    cleanup = mechanic_load_sym(modhand, cd.module, "cleanup", "master_cleanup",
        MECHANIC_MODULE_ERROR);
  } else if ((cd.mode != MECHANIC_MODE_MASTERALONE) && (node != MECHANIC_MPI_MASTER_NODE)) {
    cleanup = mechanic_load_sym(modhand, cd.module, "cleanup", "slave_cleanup",
        MECHANIC_MODULE_ERROR);
  } else {
    cleanup = mechanic_load_sym(modhand, cd.module, "cleanup", "cleanup",
        MECHANIC_MODULE_ERROR);
  }
  if (cleanup) mstat = cleanup(mpi_size, node, &md, &cd);
  mechanic_check_mstat(mstat);

  /* Free POPT */
  poptFreeContext(poptcon);

  /* Module unload */
  mechanic_module_close(modhand);

  /* HDF5 finalize */
  H5close();

  /* Cleanup LRC */
  if (node == MECHANIC_MPI_MASTER_NODE) LRC_cleanup();

  /* Mechanic cleanup */
  free(module_file);

  /* Finalize */
  mechanic_finalize(node);

  if (node == MECHANIC_MPI_MASTER_NODE) {
    mechanic_message(MECHANIC_MESSAGE_INFO, "Mechanic finished his job\n");
    mechanic_message(MECHANIC_MESSAGE_CONT, "Have a nice day!\n\n");
  }

  exit(EXIT_SUCCESS);
}

