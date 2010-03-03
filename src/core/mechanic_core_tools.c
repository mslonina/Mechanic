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
 * @page tools Core Tools
 */

#include "mechanic.h"
#include "mechanic_internals.h"

void clearArray(//< @fn Clears arrays
    MECHANIC_DATATYPE* array, //< [in, out] Array to clear
    int no_of_items_in_array) //< [in] Number of items in the array
{

	int i = 0;
	for(i = 0;i < no_of_items_in_array; i++){
		array[i] = (MECHANIC_DATATYPE)0.0;
	}

	return;
}

/**
 * @fn int* map2d()
 * @brief
 * Maps coordinates of the current pixel.
 *
 * @return
 * Array of mapped pixel
 *
 */
int* map2d(
    int c, //< [in] The number of the pixel
    void* handler, //< [in] Module handler
    moduleInfo* md, //< [in] Pointer to module info struct
    configData* d) //< [in] Pointer to config data struct
{
   
  int* ind = malloc(3*sizeof(*ind));
   int x, y;
   x = d->xres;
   y = d->yres;
   module_query_void_f qpcm;

   //we need number of current pixel to store too
   ind[2] = c;

   // Method 0: one pixel per each slave.
   if(d->method == 0){
    if(c < y) ind[0] = c / y; ind[1] = c;
    if(c > y - 1) ind[0] = c / y; ind[1] = c % y;
   }

   // Method 6: user defined control.
   if(d->method == 6){
    qpcm = load_sym(handler, md, "pixelCoordsMap", MECHANIC_MODULE_ERROR); 
    if(qpcm) qpcm(ind, c, x, y, d);
   }

   return ind;
}

int checkPixel(int pixel){
  return pixel;
}

/**
 * Wrapper to dlsym().
 * Handles error messages and abort if necessary.
 */
void* load_sym(void* handler, moduleInfo *md, char* function, int type){
 
  void* handler_f;
  char* err;
  char func[1024];

  dlerror();
  sprintf(func,"%s_%s",md->name, function);
  handler_f = dlsym(handler, func);
  
  if((err = dlerror()) != NULL){
    switch (type){
      case MECHANIC_MODULE_SILENT:
        break;
      case MECHANIC_MODULE_WARN:
        printf("-> Module warning: Cannot load function '%s': %s\n", func, err); 
        break;
      case MECHANIC_MODULE_ERROR:
        printf("-> Module error: Cannot load function '%s': %s\n", func, err); 
        break;
      default:
        break;
    }
  
    if(type == MECHANIC_MODULE_ERROR)
      MPI_Abort(MPI_COMM_WORLD, MECHANIC_ERR_MODULE);
    else
      return NULL;
    
  }else{
    return handler_f;
  }
}

/**
 * Override default popt behaviour. This code is taken from popt.h. 
 * Adjusted it only to MPI.
 */
void mechanic_displayArgs(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data){

  if(mpi_rank == 0){
    if (key->shortName == '?')
      poptPrintHelp(con, stdout, 0);
    else 
      poptPrintUsage(con, stdout, 0);
  }
  con = poptFreeContext(con);
}

void mechanic_displayUsage(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data){
    
    if (key->shortName == '?')
      help = 1;
    else
      usage = 1;

}

int mechanic_finalize(int node){

#if HAVE_MPI_SUPPORT
  MPI_Finalize();
#endif

  return 0;
}

int mechanic_abort(int errcode){

#if HAVE_MPI_SUPPORT
  MPI_Abort(MPI_COMM_WORLD, errcode);
#endif

  return 0;
}