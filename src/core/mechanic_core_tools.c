/**
 * @file
 * The Mechanic core tools
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
int map2d(int c, mechanic_internals *handler, int ind[], int** b) {

   module_query_int_f qpcm;

   do {

     /* We need number of current pixel to store, too */
     ind[2] = c;

     qpcm = mechanic_load_sym(handler, "task_coordinates_mapping", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
     if (qpcm) qpcm(ind, c, handler->config->xres, handler->config->yres, handler->info, handler->config);

     if (b[ind[0]][ind[1]] == 1) {
        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Task [%d][%d][%03d] will be skipped\n", ind[0], ind[1], ind[2]);
     } else {
        mechanic_message(MECHANIC_MESSAGE_DEBUG,
            "Task [%d][%d][%03d] will be computed\n", ind[0], ind[1], ind[2]);
     }

     c++;

   } while (b[ind[0]][ind[1]] == MECHANIC_TASK_FINISHED);

   mechanic_message(MECHANIC_MESSAGE_DEBUG,
       "Task[%d]: %d %d\n", ind[2], ind[0], ind[1]);

   /* Return number of the next pixel to be mapped */
   return c;
}
