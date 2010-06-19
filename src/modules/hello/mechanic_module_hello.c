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

/* [HELLO] */

/**
 * To understand what @M is and what it does, let us write a well-known
 * "Hello World!". We will create small C library, let us call it @c Hello
 * and save it in @c mechanic_module_hello.c file:
 *
 * @code
 * @icode modules/hello/mechanic_module_hello.c HELLOCODE
 * @endcode
 *
 * We need to compile the example code to a shared library. We can do that by
 * calling
 *
 * @code
 * gcc -fPIC -c mechanic_module_hello.c -o mechanic_module_hello.o
 * gcc -shared mechanic_module_hello.o -o libmechanic_module_hello.so
 * @endcode
 *
 * @M need to know, where our module is, so we need to adjust
 * @c LD_LIBRARY_PATH (it depends on shell you are using) to the place
 * we saved our module. If you are a @c Bash user, try the following setting
 * in your @ct .bashrc @tc file:
 *
 * @code
 * export LD_LIBRARY_PATH=/usr/lib/:/usr/local/lib:.
 * @endcode
 *
 * We run @M with our module by
 *
 * @code
 * mpirun -np 3 mechanic -p hello
 * @endcode
 *
 * This will tell @M to run on three nodes, in a task farm mode,
 * with one master node and two slaves. The master node will send default
 * initial condition (pixel coordinates) to each slave and receive data
 * in @c masterData structure (in this case the coordinates of the pixel).
 *
 * The output should be similar to:
 *
 * @code
 * -> Mechanic
 *    v. 0.12-UNSTABLE-2
 *    Author: MSlonina, TCfA, NCU
 *    Bugs: mariusz.slonina@gmail.com
 *    http://mechanics.astri.umk.pl/projects/mechanic
 * !! Config file not specified/doesn't exist. Will use defaults.
 * -> Mechanic will use these startup values:
 * (...)
 * -> Hello from slave[1]
 * -> Hello from slave[2]
 * @endcode
 *
 * Two last lines were printed using our simple module. In the working
 * directory you should find also @c mechanic-master-00.h5 file. It is a data
 * file written by the master node, and each run of @M will produce
 * such file. It containes all information about the setup of the simulation
 * and data received from slaves.
 *
 * If you try
 *
 * @code
 * h5dump -n mechanic-master-00.h5
 * @endcode
 *
 * you should see the following output:
 *
 * @code
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
 * }
 * @endcode
 *
 * which describes the data storage in master file. There are two additional
 * files in the working dir with suffixes 01 and 02 -- these are checkpoint
 * files, see @ref checkpoint for detailes.
 *
 * The @c Hello module is included in @M distribution as a simple example of
 * using the software.
 *
 * @section hello The Hello Module
 *
 * Let us go step-by-step throught the @c Hello module. Each @M module must
 * contain the preprocessor directive
 *
 * @code
 * #include "mechanic.h"
 * @endcode
 *
 * The module specific header file
 *
 * @code
 * #include "mechanic_module_hello.h"
 * @endcode
 *
 * is optional. Since each module is a normal C code, you can also use any
 * other headers and link to any other library during compilation.
 *
 * Every function in the module is prefixed with the name of
 * the module -- thus, you should use unique names for your modules. The file
 * name prefix, @c mechanic_module_ is required for proper module loading.
 *
 * The first three functions:
 *
 * - @c hello_init()
 * - @c hello_cleanup()
 * - @c hello_processPixel()
 *
 * are required for the module to work. @M will abort if any of them is
 * missing. The fourth one, @c hello_slave_out() is optional
 * and belongs to the templateable functions group
 * (see @ref template-system).
 *
 * Each function should return an integer value,
 * 0 on success and errcode on failure, which is important for proper error
 * handling.
 *
 * - @c hello_init(moduleInfo* md) is called on module initialization
 *   and you need to provide some information about the module, especially,
 *   @c md->mrl, which is the length of the results array sended from the
 *   slave node to master node. The @c moduleInfo type contains information
 *   about the module, and will be extended in the future. The structure is
 *   available for all module functions. The @c moduleInfo type has
 *   the following shape:
 *   @code
 *   @icode core/mechanic.h MODULEINFO
 *   @endcode
 *
 * - @c hello_cleanup(moduleInfo* md) currently does nothing,
 *   however, it is required for future development.
 *
 * - @c hello_processPixel(int node, moduleInfo* md, configData* d, masterData* r)
 *   is the heart of your module. Here you can compute almost any type of
 *   numerical problem or even you can call external application from here.
 *   There are technically no contradictions for including Fortran based
 *   code. In this simple example we just assign coordinates of the
 *   simulation (see @ref coords) to the result array @c r->res. The array is
 *   defined in @c masterData structure, as follows:
 *
 *   @code
 *   @icode core/mechanic.h MASTERDATA
 *   @endcode
 *
 *   Currently, @c MECHANIC_DATATYPE is set to @c double, so we need to do
 *   proper casting from integer to double. The result array
 *   has the @c md->mrl size, in this case 3. The @c masterData structure is
 *   available for all module functions.
 *
 * - @c hello_slave_out(int nodes, int node, moduleInfo* md, configData* d, masterData* r)
 *   prints formatted message from the slave node on the screen, after the
 *   node did its job. The @c mechanic_message() (see @ref devel) is available
 *   for all modules, and can be used for printing different kinds
 *   of messages, i.e. some debug information or warning.
 *
 * The @M package contains few other modules:
 *
 * - @c Module -- the default module with all available functions included
 * - @c Echo -- an extended version of the @c Hello module, which includes
 *   some advanced stuff on handling data files, and is an example of using
 *   template system
 * - @c Mandelbrot -- a benchmark module, which computes
 *   The Mandelbrot fractal.
 *
 */

/* [/HELLO] */

/**
 * @internal
 * Sample module for Mechanic
 * Keep is simple, but powerful!
 */

/* [HELLOCODE] */
#include "mechanic.h"
#include "mechanic_module_hello.h"

/**
 * Implementation of module_init()
 */
int hello_init(int mpi_size, int node, moduleInfo* md, configData* d){

  md->irl = 3;
  md->mrl = 6;

  return 0;
}

/**
 * Implementation of module_cleanup()
 */
int hello_cleanup(int mpi_size, int node, moduleInfo* md, configData* d){

  return 0;
}

int hello_master_in(int mpi_size, int node, moduleInfo* md, configData* d,
    masterData* inidata) {

  inidata->res[0] = 99.0;

  printf("Configuration test:\n");
  printf("name: %s\n", d->name);
  printf("file: %s\n", d->datafile);
  printf("module: %s\n", d->module);


  return 0;
}

int hello_master_preparePixel(int node, moduleInfo* md, configData* d,
    masterData* inidata, masterData* r) {

  inidata->res[1] = (double) d->xres;

  return 0;
}

int hello_slave_preparePixel(int node, moduleInfo* md, configData* d,
    masterData* inidata, masterData* r) {

  inidata->res[2] = (double) node;

  printf("Inidata for node %d prepared\n", node);
  return 0;
}

/**
 * Implementation of module_processPixel()
 */
int hello_processPixel(int node, moduleInfo* md, configData* d,
    masterData* inidata, masterData* r)
{

  r->res[0] = (double) r->coords[0];
  r->res[1] = (double) r->coords[1];
  r->res[2] = (double) r->coords[2];
  r->res[3] = inidata->res[0];
  r->res[4] = inidata->res[1];
  r->res[5] = inidata->res[2];

  return 0;
}

/**
 * Implementation of module_node_out()
 */
int hello_slave_out(int nodes, int node, moduleInfo* md, configData* d,
    masterData* inidata, masterData* r){

  mechanic_message(MECHANIC_MESSAGE_INFO, "Hello from slave[%d]\n", node);

  return 0;
}

/* [/HELLOCODE] */

