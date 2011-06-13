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

#include "mechanic.h"
#include "mechanic_internals.h"

/* [SETUP] */

/**
 * @section setup The Setup System
 *
 * @M uses standard configuration path -- first, we read defaults, then
 * the config file and command line options. The latter two are optional,
 * and if not present, the code will use defaults fixed at compilation time.
 *
 * To find out what command line options are available, try
 *
 * @code
 * mpirun -np 3 mechanic --help
 * @endcode
 *
 * The configuration data is available to worker nodes by the structure:
 * @code
 * @icode core/mechanic.h CONFIGDATA
 * @endcode
 *
 * @subsection cli Command Line Options
 *
 * The full list of command line options is included below:
 *
 * - @c --help @c --usage @ct -?@tc -- prints help message
 * - @c --name @c -n  -- the problem name, it will be used to prefix all data
 *   files specific in given run
 * - @c --config @c -c -- config file to use in the run
 * - @c --module @c -p -- module which should be used during the run
 * - @c --mconfig @c -m -- module config file to use in the run
 * - @c --xres @c -x -- x resolution of the simulation map
 * - @c --yres @c -y -- y resolution of the simulation map
 * - @c --checkpoint @c -d -- checkpoint file write interval
 *
 * @M provides a checkpoint system, see @ref checkpoint for
 * details. In this case the options are:
 *
 * - @c --restart @c -r -- switch to restart mode and use checkpoint file
 *
 * Mechanic can operate in different modes, see @ref modes for detailes.
 * You can switch between them by using:
 *
 * - @c -0 -- masteralone mode
 * - @c -1 -- MPI task farm mode
 *
 * @subsection configfile Config File
 *
 * @M uses @c LRC for handling config files. To load configuration from
 * custom config file use @c -c or @c --config switch. If this option is set,
 * but the file doesn't exist, @M will abort. Sample config file is
 * given below:
 *
 * @code
 * [default]
 * name = hello
 * xres = 4 #must be greater than 0
 * yres = 4 #must be greater than 0
 * module = hello 
 * mode = 1 # masteralone -- 0, task farm -- 1
 *
 * [logs]
 * checkpoint = 4
 * @endcode
 *
 * The config file options are equivalents of command line options. Any other
 * option will be silently ommited. If any of the variables is missing, @M
 * will use defaults for each not found variable. Namespaces are mandatory
 * and @M will abort if missing. The errors are handled by @c LRC in this case.
 *
 * You can include full or inline comments in your file, just after
 * the comment mark @c #. The configuration is stored in the master file,
 * see @ref storage.
 *
 * @subsection setup-examples Examples
 *
 * The general rule for running @M is to use:
 *
 * @code
 * mpirun -np NUMBER_OF_CPUS_TO_USE mechanic [OPTIONS]
 * @endcode
 *
 * Here we provide and explain some simple examples:
 * - @ct mpirun -np 4 mechanic -p mandelbrot -x 200 -y 200 -n fractal@tc <br>
 *   @M will use 4 nodes in MPI task farm mode (one master and three workers)
 *   and will compute the Mandelbrot set with resolution 200x200 pixels.
 *   The name of the run will be "fractal".
 * - @ct mpirun -np 4 mechanic -p mandelbrot -x 200 -y 200 -n fractal -0@tc <br>
 *   This is a similar example, in this case @M will compute the fractal in
 *   masteralone mode. Worker nodes will be terminated.
 * - @ct mpirun -np 4 mechanic -p mandelbrot -x 1 -y 1@tc <br>
 *   Here we can do only one simulation using the custom @c Mandelbrot module. In this
 *   case, worker nodes 2 and 3 will be terminated (see @ref modes).
 * - @ct mpirun -np 4 mechanic -p application -x 100 -y 1@tc <br>
 *   We can also create a one-dimensional simulation map, by setting one of
 *   the axes to 1. This is especially useful in non-image computations, such
 *   as observation reduction -- we can call @M to perform tasks i.e.
 *   on 100 stars.
 * - @ct mpirun -np 1 mechanic [OPTIONS]@tc <br>
 *   @ct mechanic [OPTIONS]@tc <br>
 *   @M will automatically switch to masteralone mode.
 *
 * @section coords Task-coordinate System
 *
 * @M was created for handling simulations related to dynamical maps. Thus, it
 * uses 2D task coordinate system (there are plans for extending it to other
 * dimensions). This was the simplest way to show which simulations have been
 * computed and which not. The map is stored in @c /board table in the master
 * file. Each finished simulation is marked with 1, the ongoing or broken --
 * with 0.
 *
 * It is natural to use @c (x,y)-resolution option (either in the config file
 * or command line) to describe the map of pixels for an image (like
 * a dynamical map or the Mandelbrot set). However, one can use
 * slice-based mapping, by using i.e. @c 100x1 or @c 1x100 resolution.
 * In either case, the result should be the same. Setting @ct (x,y) = (1,1)@tc
 * is equivalent of doing only one simulation.
 *
 * The mapping should help you in setting initial conditions for the
 * simulation, i.e. we can change some values by using pixel coordinates or
 * the number of the pixel. This information is available during the
 * computation and is stored in @c masterData struct.
 *
 * By default, the number of simulations is counted by multiplying x and y
 * resolution. The simulations are currently done one-by-one, the master node
 * does not participate in computations (except masteralone mode,
 * see @ref modes). 
 *
 * @section modes Modes
 *
 * @M can compute simulations both in single-cpu mode (masteralone) or
 * multi-cpu mode (MPI task farm).
 *
 * - @sb Masteralone mode@bs -- This mode is especially useful if you run @M in
 *   single-cpu environment. If the mode is used in multi-cpu environments and
 *   the size of MPI group is greater than 1, @M will terminate all nodes but
 *   the master node.
 *
 * - @sb MPI Task farm@bs -- The classical, and default mode for @M. This will
 *   use one master node and number of slave nodes to do simulations. The master
 *   node is responsible for sending/receiving data and storing them. If
 *   number of slave nodes is greater than number of simulations to do, all
 *   unused nodes will be terminated.
 *
 */

 /* [/SETUP] */

/* SETUP TOOLS */

/* Default config file parser */
int readDefaultConfig(char* inifile, int flag, LRC_configNamespace* head){

  int opts = 0;
  char* sep = "=";
  char* comm = "#";

  FILE* read;

  read = fopen(inifile, "r");

  /* Default behaviour: try to read default config file. */
  if (read != NULL) {
   mechanic_message(MECHANIC_MESSAGE_INFO,
       "Parsing config file \"%s\"... ", inifile);
   opts = LRC_ASCIIParser(read, sep, comm, head);

   if (opts >= 0) mechanic_message(MECHANIC_MESSAGE_CONT, " done.\n");
   fclose(read);
  }

  /* If -c is set, but file doesn't exist, abort. */
  else if ((read == NULL) && (flag == 1)) {
    mechanic_error(MECHANIC_ERR_SETUP);
  }

  /* We don't insist on having config file present,
   * we just use defaults instead */
  else {
    mechanic_message(MECHANIC_MESSAGE_WARN,
        "Config file not specified/doesn't exist. Will use defaults.\n");
    opts = 2;
  }

  if(opts == 0){
		mechanic_message(MECHANIC_MESSAGE_WARN,
        "Config file seems to be empty.\n");
	}

  if(opts < 0) mechanic_error(MECHANIC_ERR_SETUP);

  return opts;
}

/* Read configuration stored in the checkpoint file.
 *
 * @todo
 * return mstat, opts in function argument
 */
int readCheckpointConfig(char* file, char* group, LRC_configNamespace* head) {

  int opts = 0;
  hid_t f;
  //herr_t status;

  f = H5Fopen(file, H5F_ACC_RDONLY, H5P_DEFAULT);

  opts = LRC_HDF5Parser(f, group, head);

  //status = H5Fclose(f);
  H5Fclose(f);

  return opts;
}

/* Assign config values, one by one.
 * Final struct contains config values of the run */
int assignConfigValues(configData* d, LRC_configNamespace* head){

  char* n = NULL; char* tf = NULL; char* m = NULL; char* cm = NULL;
  size_t nlen, flen, fmlen, mlen, cmlen;

  /* Prepare the name of the problem */
  n = LRC_getOptionValue("default", "name", head);
  nlen = strlen(n);

  strncpy(d->name, n, nlen);
  d->name[nlen] = LRC_NULL;

  /* Prepare the file name */
  fmlen = strlen(MECHANIC_MASTER_SUFFIX_DEFAULT) + sizeof(char*);
  flen = nlen + fmlen + 2*sizeof(char*);

  tf = calloc(flen + sizeof(char*), sizeof(char*));
  if (tf == NULL) {
    mechanic_error(MECHANIC_ERR_MEM);
  }

  strncpy(tf, d->name, nlen);
  tf[nlen] = LRC_NULL;

  strncat(tf, MECHANIC_MASTER_SUFFIX_DEFAULT, fmlen);
  tf[flen] = LRC_NULL;

  strncpy(d->datafile, tf, flen);
  d->datafile[flen] = LRC_NULL;

  /* Prepare the module name */
  m = LRC_getOptionValue("default", "module", head);
  mlen = strlen(m);

  strncpy(d->module, m, mlen);
  d->module[mlen] = LRC_NULL;

  /* Prepare the module config file */
  cm = LRC_getOptionValue("default", "mconfig", head);
  cmlen = strlen(cm);

  strncpy(d->mconfig, cm, cmlen);
  d->mconfig[cmlen] = LRC_NULL;

  /* Other options */
	d->xres = LRC_option2int("default", "xres", head);
	d->yres = LRC_option2int("default", "yres", head);
	d->checkpoint = LRC_option2int("logs", "checkpoint", head);
	d->restartmode = 0;
	d->mode = LRC_option2int("default", "mode", head);

  /* Free resources */
  free(tf);

 return 0;
}

/* Equivalent of LRC_printAll() */
int mechanic_printConfig(configData *cd, int flag){

  if (silent == 0) {
    mechanic_message(flag, "name: %s\n", cd->name);
    mechanic_message(flag, "datafile: %s\n", cd->datafile);
    mechanic_message(flag, "module: %s\n", cd->module);
    mechanic_message(flag, "module config: %s\n", cd->mconfig);
    mechanic_message(flag, "res: [%d, %d]\n", cd->xres, cd->yres);
    mechanic_message(flag, "mode: %d\n", cd->mode);
    mechanic_message(flag, "checkpoint: %d\n", cd->checkpoint);
    mechanic_message(flag, "\n");
  }

  return 0;
}

/* Mechanic welcome message */
void mechanic_welcome(){

  mechanic_message(MECHANIC_MESSAGE_CONT, "\n");
  mechanic_message(MECHANIC_MESSAGE_INFO, "%s\n", MECHANIC_NAME);
  mechanic_message(MECHANIC_MESSAGE_CONT, "v. %s\n", MECHANIC_VERSION);
  mechanic_message(MECHANIC_MESSAGE_CONT, "Author: %s\n", MECHANIC_AUTHOR);
  mechanic_message(MECHANIC_MESSAGE_CONT, "Bugs: %s\n", MECHANIC_BUGREPORT);
  mechanic_message(MECHANIC_MESSAGE_CONT, "%s\n", MECHANIC_URL);
  mechanic_message(MECHANIC_MESSAGE_CONT, "\n");

}

/* Check if cli argument is valid */
int validate_arg(char carg[]) {
  if (carg[0] == '-' || carg[0] == '\0') {
    mechanic_message(MECHANIC_MESSAGE_WARN, "Argument '%s' in not valid.\n", carg);
    return -1;
  }

  return 0;
}
