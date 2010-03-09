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

/**
 * @page api
 * @section modules
 * @subsection module The Echo module
 * 
 * Master Data struct is defined as follows:
 * typedef struct{
 *   int count[3]; <-- handles x,y coords and number of the pixel
 *   MY_DATATYPE res[1]; <-- handles result vector, resizable with mrl variable
 * }
 *
 * @subsubsection modulecasestudies Case studies
 *  - Each slave does the same -- 
 *    This is the simplest case of using Mechanic. The only thing to do is to define 
 *    pixelCompute function and return some data to master node with masterData struct.
 *    You can also do something in functions IN/OUT, but in that case it is not really necessary.
 *
 *  - Each slave has different config file
 *    This time You need to read config file for each slave separately. This can be done with 
 *    LibReadConfig in slaveIN function and config files named after slave number, i.e. slave22.
 *
 *  - Each slave has different pixelCompute function.
 *    At this point You need to create some subfunctions of pixelCompute and choose them
 *    accordingly to number of the slave, i.e. in the switch routine.
 *
 *  - Each slave has both different config file and different pixelCompute
 *    Just combining two cases in simple switch routines and it should work too.
 *
 */

#include "mechanic.h"
#include "mechanic_module_module.h"

/**
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
 * @fn int module_init(moduleInfo* md)
 * @brief The initial function of the module.
 *
 * The init function takes care of the module info struct. You need to provide at
 * least these informations of your module to work properly, since they are
 * needed during runtime:
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

/**
 * @fn int module_farmResolution(int x, int y, moduleInfo* md, configData* d)
 * @brief Defines the resolution of the farm (method = 6)
 *
 * You can override default farm resolution mapping. Used only when method = 6.
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
int module_farmResolution(int x, int y, moduleInfo* md, configData* d){
  return x*y;
}

/**
 * @fn int module_pixelCoordsMap(int t[], int p, int x, int y, moduleInfo* md, configData* d)
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
 * Function operates on t[] pointer. Should return 0 on success, errcode otherwise.
 *
 * @ingroup module_method6
 */
int module_pixelCoordsMap(int t[], int p, int x, int y, moduleInfo* md, configData* d){
  return 0;
}

/**
 * @fn int module_pixelCoords(int node, int t[], moduleInfo* md, configData* d, masterData* r)
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
int module_pixelCoords(int node, int t[], moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_pixelCompute(int node, moduleInfo* md, configData* d, masterData* r)
 * @brief Pixel compute routine
 * 
 * The heart. Here You can compute your pixels. Possible extentions:
 * - Each node has its own pixelCompute routine. You can use them accordingly to
 *   slave number in the switch loop.
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
int module_pixelCompute(int node, moduleInfo* md, configData* d, masterData* r){
   return 0;
}

/**
 * @page template The template system
 *
 * We have some kind of a template system. Each node-prefixed function can be
 * overriden by master/slave function. The possible overrides are shown in the
 * following list:
 * - module_node_in()
 *   - module_master_in()
 *   - module_slave_in()
 * - module_node_out()
 *   - module_master_out()
 *   - module_slave_out()
 * - module_node_beforeSend()
 *   - module_master_beforeSend()
 *   - module_slave_beforeSend()
 * - module_node_afterSend()
 *   - module_master_afterSend()
 *   - module_slave_afterSend()
 * - module_node_beforeReceive()
 *   - module_master_beforeReceive()
 *   - module_slave_beforeReceive()
 * - module_node_afterReceive()
 *   - module_master_afterReceive()
 *   - module_slave_afterReceive()
 * - module_node_before_pixelCompute()
 *   - module_master_beforePixelCompute()
 *   - module_slave_beforePixelCompute()
 * - module_node_after_pixelCompute()
 *   - module_master_afterPixelCompute()
 *   - module_slave_afterPixelCompute()
 * 
 * Each template function is optional, so Mechanic will silently skip it if
 * it is missing.
 *
 */

/**
 * @fn int module_node_beforePixelCompute(int node, moduleInfo* md, configData* d, masterData* r)
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
int module_node_beforePixelCompute(int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_node_afterPixelCompute(int node, moduleInfo* md, configData* d, masterData* r)
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
int module_node_afterPixelCompute(int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_node_in(int mpi_size, int node, moduleInfo* md, configData* d)
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
 * @fn int module_node_out(int mpi_size, int node, moduleInfo* md, configData* d, masterData *r)
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
int module_node_out(int mpi_size, int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_node_beforeSend(int node, moduleInfo* md, configData* d, masterData* r)
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
int module_node_beforeSend(int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_node_afterSend(int node, moduleInfo *md, configData* d, masterData* r)
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

int module_node_afterSend(int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_node_beforeReceive(int node, moduleInfo* md, configData* d, masterData* r)
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

int module_node_beforeReceive(int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

/**
 * @fn int module_node_afterReceive(int node, moduleInfo* md, configData* d, masterData* r)
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

int module_node_afterReceive(int node, moduleInfo* md, configData* d, masterData* r){
  return 0;
}

