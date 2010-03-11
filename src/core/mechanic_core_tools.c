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
   
  uintptr_t* ind;
   int x, y;
   x = d->xres;
   y = d->yres;
   module_query_void_f qpcm;

  ind = malloc(3*sizeof(*ind));
  if(ind == NULL) mechanic_error(MECHANIC_ERR_MEM);

   //we need number of current pixel to store too
   ind[2] = c;

   // Method 0: one pixel per each slave.
   if(d->method == 0){
    if(c < y) ind[0] = c / y; ind[1] = c;
    if(c > y - 1) ind[0] = c / y; ind[1] = c % y;
   }

   // Method 6: user defined control.
   if(d->method == 6){
    qpcm = load_sym(handler, md, "pixelCoordsMap", "pixelCoordsMap", MECHANIC_MODULE_ERROR); 
    if(qpcm) qpcm(ind, c, x, y, md, d);
   }

   return ind;
}

int checkPixel(int pixel){
  return pixel;
}

/*
 * Wrapper to dlsym().
 * Handles error messages and abort if necessary.
 */
void* load_sym(void* handler, moduleInfo *md, char* function, char* function_override, int type){
 
  void* handler_f;
  void* handler_fo;
  void* ret_handler;
  char* err;
  char* err_o;
  char func[1024];
  char func_over[1024];
  int template = 0;

  dlerror();
  sprintf(func, "%s_%s", md->name, function);
  sprintf(func_over, "%s_%s", md->name, function_override);
  
  handler_f = dlsym(handler, func);
  err = dlerror();

  handler_fo = dlsym(handler, func_over);
  err_o = dlerror();
 
  // Template not found, override not found -- error check
  if(err != NULL && err_o != NULL) template = 0;

  // Template found, override not -- will use template
  if(err == NULL && err_o != NULL) template = 1;

  // Template found, override found -- will use override
  if(err == NULL && err_o == NULL) template = 2;

  // Template not found, override found -- will use override
  if(err != NULL && err_o == NULL) template = 2;

  if(template == 0){
    switch (type){
      case MECHANIC_MODULE_SILENT:
        break;
      case MECHANIC_MODULE_WARN:
        mechanic_message(MECHANIC_MESSAGE_WARN, "Module warning: Cannot load function '%s' nor its template: %s\n", func, err); 
        break;
      case MECHANIC_MODULE_ERROR:
        mechanic_message(MECHANIC_MESSAGE_ERR, "Module error: Cannot load function '%s' nor its template: %s\n", func, err); 
        break;
      default:
        break;
    }
  
    if(type == MECHANIC_MODULE_ERROR)
      mechanic_error(MECHANIC_ERR_MODULE);
    else
      return NULL;
    
  }else if (template == 1) {
    return handler_f;
  }else if (template == 2) {
    return handler_fo;
  }
}

/*
 * Override default popt behaviour. This code is taken from popt.h. 
 * Adjusted it only to MPI.
 */
void mechanic_displayArgs(int node, poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data){

  if(node == 0){
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

  MPI_Finalize();

  return 0;
}

int mechanic_abort(int errcode){

  MPI_Abort(MPI_COMM_WORLD, errcode);

  return 0;
}

void mechanic_message(int type, char *fmt, ...){

  static char fmt2[2048];
  va_list args;

  va_start(args, fmt);
    vsprintf(fmt2, fmt, args);
    if(type == MECHANIC_MESSAGE_INFO) printf("-> %s", fmt2);
    if(type == MECHANIC_MESSAGE_ERR) perror(fmt2);
    if(type == MECHANIC_MESSAGE_CONT) printf("   %s", fmt2);
		if(type == MECHANIC_MESSAGE_WARN) printf("!! %s", fmt2);
		//if(type == MECHANIC_MESSAGE_DEBUG) printf("  %s", fmt2);
  va_end(args);
 
}
