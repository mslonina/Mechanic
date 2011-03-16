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
 * Operates on array of mapped pixel
 * Returns number of next pixel to be mapped, -1 on failure
 *
 */
int map2d(int c, mechanic_internals handler, moduleInfo* md, configData* d, int ind[],
    int** b){

   int x, y;
   module_query_int_f qpcm;

   x = d->xres;
   y = d->yres;

   do {

     /* We need number of current pixel to store, too */
     ind[2] = c;

     qpcm = mechanic_load_sym(handler, "pixelCoordsMap", MECHANIC_MODULE_ERROR);
     if (qpcm) qpcm(ind, c, x, y, md, d);

     if (b[ind[0]][ind[1]] == 1) {
        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Pixel [%d][%d][%03d] will be skipped\n", ind[0], ind[1], ind[2]);
     } else {
        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Pixel [%d][%d][%03d] will be computed\n", ind[0], ind[1], ind[2]);
     }

     c++;

   } while (b[ind[0]][ind[1]] == MECHANIC_TASK_FINISHED);

   mechanic_message(MECHANIC_MESSAGE_DEBUG,
       "Pixel[%d]: %d %d\n", ind[2], ind[0], ind[1]);

   /* Return number of the next pixel to be mapped */
   return c;
}
