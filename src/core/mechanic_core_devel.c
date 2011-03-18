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

/* [DEVEL] */

/**
 * Some notes for developing the code.
 *
 * @section codingstyle Coding Style
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
 * @section message Message Interface
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

/* Wrapper to MPI_Finalize() */
int mechanic_finalize(int node){

  MPI_Finalize();

  return 0;
}

/* Wrapper to MPI_Abort() */
int mechanic_abort(int errcode){

  MPI_Abort(MPI_COMM_WORLD, errcode);

  return 0;
}

/* Mechanic message */
void mechanic_message(int type, char *fmt, ...){

  static char fmt2[2048];
  va_list args;

  va_start(args, fmt);
    vsprintf(fmt2, fmt, args);
    if (silent == 0) {
      if (type == MECHANIC_MESSAGE_INFO) printf("-> %s", fmt2);
      if (type == MECHANIC_MESSAGE_CONT) printf("   %s", fmt2);
      if (type == MECHANIC_MESSAGE_CONT2) printf("   \t\t %s", fmt2);
		  if (type == MECHANIC_MESSAGE_DEBUG && debug == 1) printf("   %s", fmt2);
    }
      if (type == MECHANIC_MESSAGE_ERR) perror(fmt2);
      if (type == MECHANIC_MESSAGE_IERR) printf("!! %s", fmt2);
		  if (type == MECHANIC_MESSAGE_WARN) printf(".. %s", fmt2);
  va_end(args);

}

/* Mechanic COPY
 * If there is a better method, please send a patch
 */
int mechanic_copy(char* in, char* out){

  int input;
  int output;
  int mstat = 0;
  char *c;
  struct stat st;

  /* Read and write files in binary mode */
  input = open(in, O_RDONLY);
  if (input < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Could not open input file %s\n", in);
    return MECHANIC_ERR_HDF;
  }

  stat(in, &st);
  mechanic_message(MECHANIC_MESSAGE_DEBUG,
      "Input file size: %d\n", (int) st.st_size);

  output = open(out, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (output < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Could not open output file %s\n", out);
    return MECHANIC_ERR_HDF;
  }

  /* CHAR_BIT in limits.h */
  c = malloc(st.st_size * CHAR_BIT + 1);
  if (c == NULL) return MECHANIC_ERR_MEM;

  /* Read and write the whole file, without loops */
  mstat = read(input, c, st.st_size);
  if (mstat < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Could not read input file %s\n", in);
    return MECHANIC_ERR_HDF;
  }

  mstat = write(output, c, st.st_size);
  if (mstat < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Could not write output file %s\n", out);
    return MECHANIC_ERR_HDF;
  }

  free(c);

  if (close(input) < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Error closing input file\n");
    return MECHANIC_ERR_HDF;
  }

  if (close(output) < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Error closing output file\n");
    return MECHANIC_ERR_HDF;
  }

  return mstat;
}

/* Convert 2D integer array to 1D integer vector */
int* IntArrayToVec(int** array, int x, int y) {

  int* vec;
  int i, j, k, size;

  size = x * y;
  vec = AllocateIntVec(size);

  for(i = 0; i < x; i++) {
    k = i * y;
    for(j = 0; j < y; j++) {
      vec[j+k] = array[i][j];
    }
  }

  return vec;
}

int* AllocateIntVec(int x) {

  int* vec;

  vec = calloc(sizeof(uintptr_t) * x, sizeof(uintptr_t));
  if(vec == NULL) mechanic_error(MECHANIC_ERR_MEM);

  return vec;
}

void FreeIntVec(int* vec) {
  if(vec) free(vec);
}

/* Convert 2D double array to 1D vector */
double* DoubleArrayToVec(double** array, int x, int y) {

  double* vec;
  int i, j, k, size;

  size = x * y;
  vec = AllocateDoubleVec(size);

  for(i = 0; i < x; i++) {
    k = i * y;
    for(j = 0; j < y; j++) {
      vec[j+k] = array[i][j];
    }
  }

  return vec;
}

double* AllocateDoubleVec(int x) {
  double* vec;

  vec = calloc(sizeof(double) * x, sizeof(double));
  if (vec == NULL) mechanic_error(MECHANIC_ERR_MEM);

  return vec;
}

void FreeDoubleVec(double* vec) {
  if(vec) free(vec);
}

/* Allocate 2D int array */
int** AllocateInt2D(int x, int y) {

  int** pointer;
  int i;

  pointer = (int**) AllocateIntVec(x);

  if (pointer) {
    for (i = 0; i < x; i++) {
      pointer[i] = calloc(sizeof(uintptr_t) * y, sizeof(uintptr_t));
      if (pointer[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  return pointer;
}

double** AllocateDouble2D(int x, int y) {

  double** pointer;
  int i;

  pointer = (double**) AllocateDoubleVec(x);

  if (pointer) {
    for(i = 0; i < x; i++) {
      pointer[i] = calloc(sizeof(double) * y, sizeof(double));
      if(pointer[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  return pointer;
}

void FreeDouble2D(double** pointer, int elems) {
  int i = 0;
  for (i = 0; i < elems; i++) {
    if(pointer[i]) free(pointer[i]);
  }
  if(pointer) free(pointer);
}

void FreeInt2D(int** pointer, int elems) {
  int i = 0;
  for (i = 0; i < elems; i++) {
    if (pointer[i]) free(pointer[i]);
  }
  if(pointer) free(pointer);
}

int mechanic_ups() {
  return 0;
}
