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

/* [CASES] */

/**
 * @section modulecasestudies Case studies
 *
 * There are some basic use cases of @M and its template system:
 *
 *  - @sb Each slave does the same. @bs
 *    This is the simplest case of using @M. The only thing to do is to
 *    define @c pixelCompute() function and return data to the master node
 *    with @c masterData structure. You can also do something more in
 *    @c node_in/out functions, but in that case it is not really necessary.
 *
 *  - @sb Each slave has different config file. @bs
 *    This time you need to read config file for each slave separately.
 *    This can be done with @c LRC in @c slave_in() function and config files
 *    named after slave number, i.e. slave22.
 *
 *  - @sb Each slave has different @c pixelCompute function. @bs
 *    At this point you need to create some subfunctions of @c pixelCompute
 *    and choose them accordingly to number of the slave, i.e.
 *    in the switch routine.
 *
 *  - @sb Each slave has both different config file and different
 *    @c pixelCompute. @bs
 *    Just combining two cases in simple switch routines and it should work.
 *
 */

/* [/CASES] */

#include "mechanic.h"
#include "mechanic_module_module.h"

/**
 * @internal
 * @defgroup module_required Required module functions
 * @{
 * @}
 *
 * @defgroup module_themeable Themeable module functions
 * @{
 * @}
 *
 * @defgroup module_method6 Required when method = 6
 * @{
 * @}
 */

/**
 * @internal
 * @fn int module_init(moduleInfo* md)
 * @brief The initial function of the module.
 *
 * The init function takes care of the module info struct.
 * You need to provide at least these informations of your module to work
 * properly, since they are needed during runtime:
 *
 * @code
 * md->name
 * md->author
 * md->date
 * md->version
 * md->mrl
 * @endcode
 *
 * @param *md
 * Pointer to the module info struct.
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_required
 */
int module_init(moduleInfo* md){

  md->name = "module";
  md->author = "MSlonina";
  md->date = "2010";
  md->version = "1.0";
  md->mrl = 4;

  return 0;
}

/**
 * @internal
 * @fn int module_query(moduleInfo* md)
 * @brief The custom query of the module.
 *
 * @param *md
 * The pointer to the module info struct.
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_optional
 */
int module_query(moduleInfo* md){
  return 0;
}

/**
 * @internal
 * @fn int module_cleanup(moduleInfo* md)
 * @brief The cleanup functions of the module.
 *
 * @param *md
 * The pointer to the module info struct.
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_required
 */
int module_cleanup(moduleInfo* md){
  return 0;
}

/* [METHOD6] */
/**
 * @section method6 The Method 6
 *
 * You can change default @M pixel mapping and simulation handling by setting
 * @ct method = 6 @tc. In this case, you need to provide additional functions
 * in your module. If any of them is missing, @M will abort.
 *
 * The @c farmResolution() simply returns number of simulations to do.
 *
 * @code
 * @icode modules/mechanic_module_module.c 6FARM
 * @endcode
 *
 * The @c pixelCoordsMap() operates on @c t index and should return 0 on
 * success, errcode otherwise. The default behaviour is to map pixels on 2D
 * board, as shown below:
 *
 * @code
 * @icode modules/mechanic_module_module.c 6COORDS
 * @endcode
 *
 * The @c pixelCoords() assigns pixel coordinates to @ct masterData r@tc
 * structure. The default behaviour is to copy @c t index to @ct r->coords @tc.
 *
 * @code
 * @icode modules/mechanic_module_module.c 6PIXELS
 * @endcode
 *
 */
/* [/METHOD6] */

/**
 * @internal
 * @fn int module_farmResolution(int x, int y, moduleInfo* md, configData* d)
 * @brief Defines the resolution of the farm (method = 6)
 *
 * You can override default farm resolution mapping.
 * Used only when method = 6.
 *
 * @param x
 * The x resolution of the farm (spool)
 *
 * @param y
 * The y resolution of the farm (spool)
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data
 *
 * @return
 * Farm resolution on success, errcode otherwise
 *
 * @ingroup module_method6
 */

/* [6FARM] */

int module_farmResolution(int x, int y, moduleInfo* md, configData* d){

  return x*y;
}

/* [/6FARM] */

/**
 * @internal
 * @fn int module_pixelCoordsMap(int t[], int p, int x, int y, moduleInfo* md,
 * configData* d)
 * @brief Defines pixel mapping in the farm (method = 6)
 *
 * You can overwrite pixel coords alignment here. Used only when method = 6.
 * Default is to create a map of pixels -- each pixel has (x,y) coordinates.
 *
 * @param t
 * Coords array, sent to each slave.
 *
 * @param p
 * The number of current pixel
 *
 * @param x
 * The x resolution of the farm (spool)
 *
 * @param y
 * The y resolution of the farm (spool)
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @return
 * Function operates on t[] pointer. Should return 0 on success,
 * errcode otherwise.
 *
 * @ingroup module_method6
 */

/* [6COORDS] */

int module_pixelCoordsMap(int t[], int numofpx, int xres, int yres, moduleInfo* md,
    configData* d){

  if (numofpx < yres) {
    t[0] = numofpx / yres;
    t[1] = numofpx;
  }

  if (numofpx > yres - 1) {
    t[0] = numofpx / yres;
    t[1] = numofpx % yres;
  }

  return 0;
}

/* [/6COORDS] */

/**
 * @internal
 * @fn int module_pixelCoords(int node, int t[], moduleInfo* md,
 * configData* d, masterData* r)
 * @brief Pixel coords mapping
 *
 * Each slave takes the pixel coordinates and then do its work.
 * Here You can change pixel assignment to output masterData r.
 * Used only when method = 6.
 *
 * Default assignment is to copy t[] values to master result array:
 * @code
 * r->coords[0] = t[0] //x
 * r->coords[1] = t[1] //y
 * r->coords[2] = t[2] //number of the pixel
 * @endcode
 *
 * @param node
 * The number of node
 *
 * @param t
 * The array of mapped pixels
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master results struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_method6
 */

/* [6PIXELS] */

int module_pixelCoords(int node, int t[], moduleInfo* md, configData* d,
    masterData* r){

  r->coords[0] = t[0];
  r->coords[1] = t[1];
  r->coords[2] = t[2];

  return 0;
}

/* [/6PIXELS] */

/**
 * @internal
 * @fn int module_pixelCompute(int node, moduleInfo* md, configData* d,
 * masterData* r)
 * @brief Pixel compute routine
 *
 * The heart. Here You can compute your pixels. Possible extentions:
 * - Each node has its own pixelCompute routine. You can use them accordingly
 *   to slave number in the switch loop.
 * - Each node has the same pixelCompute routine.
 *
 * @param node
 * The number of current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_required
 */
int module_pixelCompute(int node, moduleInfo* md, configData* d,
    masterData* r){
   return 0;
}

/* [TEMPLATES] */

/**
 * @section template-system The template system
 *
 * @M uses some kind of a template system. It allows developer to use
 * different sets of functions at different modes. Below we present list of
 * available template functions and their possible overrides. For sake of
 * clearness, we omitted function's arguments.
 *
 * - @ct module_node_in(int mpi_size, int node, moduleInfo* md, configData* d) @tc
 *   - @c module_master_in()
 *   - @c module_slave_in()
 * - @ct module_node_out(int mpi_size, int node, moduleInfo* md, configData* d) @tc
 *   - @c module_master_out()
 *   - @c module_slave_out()
 * - @ct module_node_beforeSend(int node, moduleInfo* md, configData* d,
 *   masterData* r) @tc
 *   - @c module_master_beforeSend()
 *   - @c module_slave_beforeSend()
 * - @ct module_node_afterSend(int node, moduleInfo* md, configData* d,
 *   masterData* r) @tc
 *   - @c module_master_afterSend()
 *   - @c module_slave_afterSend()
 * - @ct module_node_beforeReceive(int node, moduleInfo* md, configData* d,
 *   masterData* r) @tc
 *   - @c module_master_beforeReceive()
 *   - @c module_slave_beforeReceive()
 * - @ct module_node_afterReceive(int node, moduleInfo* md, configData* d,
 *   masterData* r) @tc
 *   - @c module_master_afterReceive()
 *   - @c module_slave_afterReceive()
 * - @ct module_node_before_pixelCompute(int node, moduleInfo* md, configData*
 *   d, masterData* r) @tc
 *   - @c module_master_beforePixelCompute()
 *   - @c module_slave_beforePixelCompute()
 * - @ct module_node_after_pixelCompute(int node, moduleInfo* md, configData*
 *   d, masterData* r) @tc
 *   - @c module_master_afterPixelCompute()
 *   - @c module_slave_afterPixelCompute()
 *
 * Each template function is optional, so @M will silently skip it if
 * it is missing. Refer to @ref echo for a simple example of using
 * the template system.
 *
 */

/* [/TEMPLATES] */

/**
 * @internal
 * @fn int module_node_beforePixelCompute(int node, moduleInfo* md,
 * configData* d, masterData* r)
 * @brief Operates on data before pixel computation
 *
 * This function can be used before pixel computation.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */
int module_node_beforePixelCompute(int node, moduleInfo* md, configData* d,
    masterData* r){
  return 0;
}

/**
 * @internal
 * @fn int module_node_afterPixelCompute(int node, moduleInfo* md,
 * configData* d, masterData* r)
 * @brief Operates on data after pixel computation
 *
 * This function can be used after pixel computation.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */
int module_node_afterPixelCompute(int node, moduleInfo* md, configData* d,
    masterData* r){
  return 0;
}

/**
 * @internal
 * @fn int module_node_in(int mpi_size, int node, moduleInfo* md,
 * configData* d)
 * @brief Called before any farm operations.
 *
 * You can do something before computations starts.
 *
 * @param mpi_size
 * The size of the spool. In masteralone mode set to 1.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */
int module_node_in(int mpi_size, int node, moduleInfo* md, configData* d){
  return 0;
}

/**
 * @internal
 * @fn int module_node_out(int mpi_size, int node, moduleInfo* md,
 * configData* d, masterData *r)
 * @brief Called after all farm operations.
 *
 * You can do something after computations finish.
 *
 * @param mpi_size
 * The size of the spool. In masteralone mode set to 1.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */
int module_node_out(int mpi_size, int node, moduleInfo* md, configData* d,
    masterData* r){
  return 0;
}

/**
 * @internal
 * @fn int module_node_beforeSend(int node, moduleInfo* md, configData* d,
 * masterData* r)
 * @brief Called before send data to slaves.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */
int module_node_beforeSend(int node, moduleInfo* md, configData* d,
    masterData* r){
  return 0;
}

/**
 * @internal
 * @fn int module_node_afterSend(int node, moduleInfo *md, configData* d,
 * masterData* r)
 * @brief Called after data was send to slaves.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */

int module_node_afterSend(int node, moduleInfo* md, configData* d,
    masterData* r)
{
  return 0;
}

/**
 * @internal
 * @fn int module_node_beforeReceive(int node, moduleInfo* md,
 * configData* d, masterData* r)
 * @brief Called before data receive from the slave.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */

int module_node_beforeReceive(int node, moduleInfo* md, configData* d,
    masterData* r){
  return 0;
}

/**
 * @internal
 * @fn int module_node_afterReceive(int node, moduleInfo* md, configData* d,
 * masterData* r)
 * @brief Called after data is received.
 *
 * @param node
 * The number of the current node
 *
 * @param *md
 * The pointer to the module info struct
 *
 * @param *d
 * The pointer to the configuration data struct
 *
 * @param *r
 * The pointer to the master result struct
 *
 * @return
 * Should return 0 on success, errcode otherwise
 *
 * @ingroup module_themeable
 */

int module_node_afterReceive(int node, moduleInfo* md, configData* d,
    masterData* r){
  return 0;
}

