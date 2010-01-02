#include "mpifarm.h"

/**
 * HELPER FUNCTIONS
 */

/**
 * Clears arrays.
 */
void clearArray(MY_DATATYPE* array, int no_of_items_in_array){

	int i;
	for(i = 0;i < no_of_items_in_array; i++){
		array[i] = (MY_DATATYPE)0.0;
	}

	return;
}


/**
 * Map 1D index to 2D array.
 */

int* map2d(int c, void* module, configData *d){
   int *ind = malloc(3*sizeof(*ind));
   int x, y;
   x = d->xres;
   y = d->yres;
   module_query_void_f qpcm;
  
   ind[2] = c; //we need number of current pixel to store too

   /**
    * Method 0: one pixel per each slave.
    */
   if(d->method == 0){
    if(c < y) ind[0] = c / y; ind[1] = c;
    if(c > y - 1) ind[0] = c / y; ind[1] = c % y;
   }

   /**
    * Method 6: user defined control.
    */
   if(d->method == 6){
    qpcm = load_sym(module, "userdefined_pixelCoordsMap", MODULE_ERROR); 
    if(qpcm) qpcm(ind, c, x, y);
   }

   return ind;
}

/**
 * Wrapper to load_sym().
 * Handles error messages and abort if necessary.
 */
void* load_sym(void* module, char* function, int rank){
 
  void* handler;
  char* err;

  dlerror();
  handler = dlsym(module, function);
  if((err = dlerror()) != NULL){
    switch (rank){
      case MODULE_SILENT:
        break;
      case MODULE_WARN:
        printf("Module warning: Cannot load function '%s': %s\n", function, err); 
        break;
      case MODULE_ERROR:
        printf("Module error: Cannot load function '%s': %s\n", function, err); 
        break;
      default:
        break;
    }
    if(rank == MODULE_ERROR){
      MPI_Abort(MPI_COMM_WORLD, 913);
    }else{
      return NULL;
    }
  }else{
    return handler;
  }
}

/**
 * HDF5 error handling.
 */
void H5errcheck(hid_t handler, herr_t stat, char* message, int rank){
  return;
}
