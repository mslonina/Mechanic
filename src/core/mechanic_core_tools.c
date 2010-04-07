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

/* [DEVEL] */

/**
 * Some notes for developing the code.
 *
 * @section codingstyle Coding style
 *
 * - We try to follow ANSI C standard as close as possible, however we use
 *   also some advantages of C99. Thus, you should use ANSI C where possible,
 *   and C99 where necessary.
 * - We use ANSI C comment style. Because of Doxygen documenting style, use
 *   one asteriks @c * at start of your non-documentation comments.
 * - We use 2 spaces indenting style (you can easily map tabs to 2 spaces).
 * - Try not to use globals. Avoid them where possible.
 * - Remember, the code should be readable by humans. For the machines,
 *   it does not matter.
 * - We follow @c PEAR coding standards, see
 *   http://pear.php.net/manual/en/standards.php
 *
 * If you are a lucky vim user, try settings below:
 * @code
 * :set textwidth=79
 * :set shiftwidth=2
 * :set tabstop=2
 * :set smarttab
 * :set expandtab
 * :set list
 * :highlight OverLength ctermbg=red ctermfg=white guibg=#592929
 * :match OverLength /\%80v.* /
 * :let c_space_errors = 1
 * @endcode
 *
 * @section message Message interface
 *
 * @M provides some functions that should be used instead of standard
 * @c printf and @c exit:
 *
 * - @code void mechanic_message(int type, char* fmt, ...) @endcode
 *   A wrapper for any message printing. The available types are:
 *   - @c MECHANIC_MESSAGE_INFO -- information style
 *   - @c MECHANIC_MESSAGE_CONT -- continuation of any message style
 *   (when the message is too long, and you want to split it)
 *   - @c MECHANIC_MESSAGE_ERR -- error only
 *   - @c MECHANIC_MESSAGE_WARN -- warning only
 *   - @c MECHANIC_MESSAGE_DEBUG -- debug only
 *
 * The @c *fmt is a standard @c printf format, and dots @c ... are arguments
 * for it.
 *
 * - @code mechanic_error(int stat) @endcode
 *   Handles error states, see @ref errcodes for available error codes.
 *
 * - @code mechanic_abort(int node) @endcode
 *   A wrapper for @c MPI_Abort.
 *
 * - @code mechanic_finalize(int node) @endcode
 *   A wrapper for @c MPI_Finalize.
 *
 */

/* [/DEVEL] */

#include "mechanic.h"
#include "mechanic_internals.h"

void clearArray(MECHANIC_DATATYPE* array, int no_of_items_in_array){

	int i = 0;

	for (i = 0;i < no_of_items_in_array; i++) {
		array[i] = (MECHANIC_DATATYPE) 0.0;
	}

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
int map2d(int c, void* handler, moduleInfo* md, configData* d, int ind[]){

   int x, y;
   module_query_void_f qpcm;

   x = d->xres;
   y = d->yres;

   /* We need number of current pixel to store, too */
   ind[2] = c;

   /* Method 0: one pixel per each slave. */
   if (d->method == 0) {

    if (c < y) {
      ind[0] = c / y;
      ind[1] = c;
    }

    if (c > y - 1) {
      ind[0] = c / y;
      ind[1] = c % y;
    }
   }

   /* Method 6: user defined pixel mapping. */
   if (d->method == 6) {
    qpcm = load_sym(handler, md, "pixelCoordsMap", "pixelCoordsMap",
        MECHANIC_MODULE_ERROR);
    if (qpcm) qpcm(ind, c, x, y, md, d);
   }

   mechanic_message(MECHANIC_MESSAGE_DEBUG,
       "Pixel[%d]: %d %d\n", ind[2], ind[0], ind[1]);

   return 0;
}

int checkPixel(int pixel){

  return pixel;
}

/*
 * Wrapper to dlsym().
 * Handles error messages and abort if necessary.
 */
void* load_sym(void* handler, moduleInfo *md, char* function,
    char* function_override, int type){

  void* handler_f;
  void* handler_fo;
  void* ret_handler;
  char* err;
  char* err_o;
  char* func;
  char* func_over;
  int template = 0;
  size_t fl, fol, mn;

  /* Reset dlerror() */
  dlerror();

  mn = strlen(md->name);
  fl = strlen(function);
  fol = strlen(function_override);

  func = calloc(mn + fl + 2 * sizeof(char*), sizeof(char*));
  if (func == NULL) mechanic_error(MECHANIC_ERR_MEM);
  
  func_over = calloc(mn + fol + 2 * sizeof(char*), sizeof(char*));
  if (func_over == NULL) mechanic_error(MECHANIC_ERR_MEM);

  sprintf(func, "%s_%s", md->name, function);
  sprintf(func_over, "%s_%s", md->name, function_override);

  handler_f = dlsym(handler, func);
  err = dlerror();

  handler_fo = dlsym(handler, func_over);
  err_o = dlerror();

  /* Template not found, override not found -- error check */
  if (err != NULL && err_o != NULL) template = 0;

  /* Template found, override not -- will use template */
  if (err == NULL && err_o != NULL) template = 1;

  /* Template found, override found -- will use override */
  if (err == NULL && err_o == NULL) template = 2;

  /* Template not found, override found -- will use override */
  if (err != NULL && err_o == NULL) template = 2;

  if (template == 0) {
    switch (type) {
      case MECHANIC_MODULE_SILENT:
        break;
      case MECHANIC_MODULE_WARN:
        mechanic_message(MECHANIC_MESSAGE_WARN,
            "Module warning: Cannot load function '%s' nor its template: %s\n",
            func, err);
        break;
      case MECHANIC_MODULE_ERROR:
        mechanic_message(MECHANIC_MESSAGE_ERR,
            "Module error: Cannot load function '%s' nor its template: %s\n",
            func, err);
        break;
      default:
        break;
    }

    if (type == MECHANIC_MODULE_ERROR)
      mechanic_error(MECHANIC_ERR_MODULE);
    else
      return NULL;

  } else if (template == 1) {
    ret_handler = handler_f;
  } else if (template == 2) {
    ret_handler = handler_fo;
  }
  
  free(func);
  free(func_over);

  return ret_handler;
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
    if (silent == 0) {
      if (type == MECHANIC_MESSAGE_INFO) printf("-> %s", fmt2);
      if (type == MECHANIC_MESSAGE_CONT) printf("   %s", fmt2);
		  if (type == MECHANIC_MESSAGE_DEBUG && debug == 1) printf("   %s", fmt2);
    }
      if (type == MECHANIC_MESSAGE_ERR) perror(fmt2);
		  if (type == MECHANIC_MESSAGE_WARN) printf(".. %s", fmt2);
  va_end(args);

}

int mechanic_printConfig(configData *cd, int flag){

  if (silent == 0) {
    mechanic_message(flag, "name: %s\n", cd->name);
    mechanic_message(flag, "datafile: %s\n", cd->datafile);
    mechanic_message(flag, "module: %s\n", cd->module);
    mechanic_message(flag, "res: [%d, %d]\n", cd->xres, cd->yres);
    mechanic_message(flag, "mode: %d\n", cd->mode);
    mechanic_message(flag, "method: %d\n", cd->method);
    mechanic_message(flag, "checkpoint: %d\n", cd->checkpoint);
    mechanic_message(flag, "\n");
  }

  return 0;
}

void mechanic_welcome(){

  mechanic_message(MECHANIC_MESSAGE_INFO, "%s\n", MECHANIC_NAME);
  mechanic_message(MECHANIC_MESSAGE_CONT, "v. %s\n", MECHANIC_VERSION);
  mechanic_message(MECHANIC_MESSAGE_CONT, "Author: %s\n", MECHANIC_AUTHOR);
  mechanic_message(MECHANIC_MESSAGE_CONT, "Bugs: %s\n", MECHANIC_BUGREPORT);
  mechanic_message(MECHANIC_MESSAGE_CONT, "%s\n", MECHANIC_URL);
  mechanic_message(MECHANIC_MESSAGE_CONT, "\n");

}

