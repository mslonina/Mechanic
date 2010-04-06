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

#include "mechanic.h"
#include "mechanic_internals.h"

/* [SETUP] */

/**
 * @section setup The setup system
 *
 * @M uses standard configuration path -- we read defaults, config file and
 * commandline options. The latter two are optional, and if not present, the
 * code will use defaults fixed at compilation time.
 *
 * To find out what commandline options are available, try
 *
 * @code
 * mpirun -np 3 mechanic --help
 * @endcode
 *
 * @subsection cli Commandline options
 *
 * The full list of commandline options is included below:
 *
 * - @c --help @c --usage @c -? -- prints help message
 * - @c --name @c -n  -- the problem name, it will be used to prefix all data
 *   files specific for given run
 * - @c --config @c -c -- config file to use for the run
 * - @c --module @c -p -- module which should be used during the run
 * - @c --method @c -m -- method of pixel mapping (0 -- default,
 *   6 -- user-defined @c [TODO])
 * - @c --xres @c -x -- x resolution of the simulation map
 * - @c --yres @c -y -- y resolution of the simulation map
 * - @c --checkpoint @c -d -- checkpoint write interval
 *
 * Restart (checkpoints) options are @c [TODO]:
 *
 * - @c --restart @c -r -- switch to restart mode
 * - @c --rpath @c -b -- checkpoint file path
 *
 * Mechanic can operate in different modes, see @ref modes for detailes.
 * You can switch between them by using:
 *
 * - @c -0 -- masteralone mode
 * - @c -1 -- task farm mode
 * - @c -2 -- multi task farm mode @c [TODO]
 *
 * @subsection configfile Config file
 *
 * Mechanic uses @c LRC for handling config files. To load configuration from
 * custom config file use @c -c switch. If this option is set,
 * but the file doesn't exist, Mechanic will abort. Sample config file is
 * given below:
 *
 * @code
 * [default]
 * name = hello
 * xres = 4 #must be greater than 0
 * yres = 4 #must be greater than 0
 * method = 0 #single pixel -- 0, userdefined -- 6
 * module = hello # modules: hello, echo, mandelbrot, module
 * mode = 1 # masteralone -- 0, task farm -- 1, multi task farm -- 2
 *
 * [logs]
 * checkpoint = 4
 * @endcode
 *
 * The config file options are equivalents of commandline options. Any other
 * option will be silently ommited. If any of variables is missing, Mechanic
 * will use defaults. Namespaces are mandatory and Mechanic will abort
 * if missing.
 *
 * You can include full or inline comments in your file, just after
 * the comment mark @c #. The configuration is stored in master file,
 * see @ref storage.
 *
 * @subsection setup-examples Examples
 *
 * The general rule for running @M is to use:
 *
 * @code
 * mpirun -np NUM_OF_CPU mechanic [OPTIONS]
 * @endcode
 *
 * Examples:
 * - @ct mpirun -np 4 mechanic -p mandelbrot -x 2000 -y 2000 -n fractal @tc 
 *   @M will use 4 nodes in MPI task farm mode (one master and three slaves)
 *   and will compute the Mandelbrot fractal with resolution 2000x2000 pixels.
 *   The name of the run will be "fractal".
 * - @ct mpirun -np 4 mechanic -p mandelbrot -x 2000 -y 2000 -n fractal -0 @tc
 *   This is a similar example, in this case @M will compute the fractal in
 *   masteralone mode. Slave nodes will be terminated.
 * - @ct mpirun -np 4 mechanic -p mandelbrot -x 1 -y 1 @tc
 *   Here we can do only one simulation using the "Mandelbrot" module. In this
 *   case, slave nodes 2 and 3 will be terminated (see @ref modes).
 * - @ct mpirun -np 4 mechanic -p application -x 100 -y 1 @tc
 *   We can also create a one-dimensional simulation map, by setting one of
 *   axes to 1.
 * - @ct mpirun -np 1 mechanic [OPTIONS] @tc
 *   @ct mechanic [OPTIONS] @tc
 *   @M will automatically switch to masteralone mode.
 *
 * @section coords Pixel-Coordinate system
 *
 * @M was created for handling simulations related to dynamical maps. Thus, it
 * uses 2D pixel coordinate system (there are plans for extending it to other
 * dimensions). This was the easiest way to show which simulations have been
 * computed and which no. The map is stored in @c /board table in the master
 * file. Each finished simulation is marked with 1, the ongoing or broken --
 * with 0.
 *
 * It is natural to use (x,y)-resolution option (either in the config file or
 * commandline) to describe the map of pixels for an image (like dynamical
 * map or the Mandelbrot fractal). However, one can use slice-based mapping,
 * by using i.e. 100x1 or 1x100 resolution. In either case, the result should
 * be the same. Setting (x,y) = (1,1) is equivalent for doing only one
 * simulation.
 *
 * The mapping should help you in setting initial conditions for the
 * simulation, i.e. we can change some values by using pixel coordinates or
 * the number of the pixel. This information is available during the
 * computation and stored in @c masterData struct.
 *
 * The number of simulations is counted by multiplying x and y resolution.
 * The simulations are currently done one-by-one, the master node does not
 * participate in computations (except masteralone mode, see @ref modes).
 *
 * @section modes Modes
 *
 * @M can compute simulations both in single-cpu mode (masteralone) or
 * multi-cpus mode (MPI task farm).
 *
 * - Masteralone mode -- This mode is especially useful if you run @M in
 *   single-cpu environment. If the mode is used in multi-cpu environments and
 *   the size of MPI group is greater than 1, @M will terminate all nodes but
 *   the master node.
 *
 * - MPI Task farm -- The classical, and default mode for @M. This will use
 *   one master node and number of slave nodes to do simulations. The master
 *   node is responsible for sending/receiving data and storing them. If
 *   number of slave nodes is greater than number of simulations to do, all
 *   unused nodes will be terminated.
 *
 */

 /* [/SETUP] */

/* SETUP TOOLS */

/* Default config file parser */
int readDefaultConfig(char* inifile, int flag){

  int opts = 0;
  char* sep = "=";
  char* comm = "#";

  FILE* read;

  read = fopen(inifile, "r");

  /* Default behaviour: try to read default config file. */
  if (read != NULL) {
   mechanic_message(MECHANIC_MESSAGE_INFO,
       "Parsing config file \"%s\"... ", inifile);
   opts = LRC_ASCIIParser(read, sep, comm);

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

/* Assign config values, one by one.
 * Final struct contains config values of the run */
int assignConfigValues(configData* d){

  char* n = NULL; char* tf = NULL; char* m = NULL;
  size_t nlen, flen, fmlen, mlen;
  char* tempaddr = NULL;

  /* Prepare the name of the problem */
  n = LRC_getOptionValue("default", "name");
  nlen = strlen(n);

  if (d->name == NULL) {
    d->name = malloc(nlen + sizeof(char*));
    if (d->name == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  } else {
    tempaddr = realloc(d->name, nlen + sizeof(char*));
    if (d->name == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  strncpy(d->name, n, nlen);
  d->name[nlen] = LRC_NULL;

  /* Prepare the file name */
  fmlen = strlen(MECHANIC_MASTER_SUFFIX_DEFAULT) + sizeof(char*);
  flen = nlen + fmlen + 2*sizeof(char*);

  tf = malloc(flen + sizeof(char*));
  if (tf == NULL) {
    mechanic_error(MECHANIC_ERR_MEM);
  }

  strncpy(tf, d->name, nlen);
  tf[nlen] = LRC_NULL;

  strncat(tf, MECHANIC_MASTER_SUFFIX_DEFAULT, fmlen);
  tf[flen] = LRC_NULL;

  if (d->datafile == NULL) {
    d->datafile = malloc(flen + sizeof(char*));
    if (d->datafile == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  } else {
    tempaddr = realloc(d->datafile, flen + sizeof(char*));
    if (d->datafile == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  strncpy(d->datafile, tf, flen);
  d->datafile[flen] = LRC_NULL;

  /* Prepare the module name */
  m = LRC_getOptionValue("default", "module");
  mlen = strlen(m);

  if (d->module == NULL) {
    d->module = malloc(nlen + sizeof(char*));
    if (d->module == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  } else {
    tempaddr = realloc(d->module, nlen + sizeof(char*));
    if (d->module == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  strncpy(d->module, m, mlen);
  d->module[mlen] = LRC_NULL;

  /* Other options */
	d->xres = LRC_option2int("default", "xres");
	d->yres = LRC_option2int("default", "yres");
	d->method = LRC_option2int("default", "method");
	d->checkpoint = LRC_option2int("logs", "checkpoint");
	d->restartmode = 0;
	d->mode = LRC_option2int("default", "mode");

  /* Free resources */
  free(tf);

 return 0;
}

