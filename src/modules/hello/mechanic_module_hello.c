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

#include "mechanic.h"
#include "mechanic_module_hello.h"

/**
 * Sample module for Mechanic
 * Keep is simple, but powerful!
 */

/**
 * Implementation of module_init()
 */
int hello_init(moduleInfo* md){

  md->author = "Mariusz Slonina";
  md->date = "2010";
  md->version = "1.0";
  md->mrl = 3;
  
  return 0;
}

/**
 * Implementation of module_cleanup()
 */
int hello_cleanup(moduleInfo* md){
  return 0;
}

/**
 * Implementation of module_pixelCompute()
 */
int hello_pixelCompute(int node, moduleInfo* md, configData* d, masterData* r){

  r->res[0] = (double)r->coords[0];
  r->res[1] = (double)r->coords[1];
  r->res[2] = (double)r->coords[2];
  
  return 0;
}

/**
 * Implementation of module_node_out()
 */
int hello_slave_out(int nodes, int node, moduleInfo* md, configData* d, masterData* r){
  mechanic_message(MECHANIC_MESSAGE_INFO, "Hello from slave[%d]\n", node);
  return 0;
}
