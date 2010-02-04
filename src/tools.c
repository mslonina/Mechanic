#include "mechanic.h"
#include "mechanic-internals.h"

/**
 * HELPER FUNCTIONS
 */

/* Clears arrays */
void clearArray(MECHANIC_DATATYPE* array, int no_of_items_in_array){

	int i = 0;
	for(i = 0;i < no_of_items_in_array; i++){
		array[i] = (MECHANIC_DATATYPE)0.0;
	}

	return;
}

/* Map 1D index to 2D array */
int* map2d(int c, void* handler, moduleInfo *md, configData* d){
   
  int* ind = malloc(3*sizeof(*ind));
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
    qpcm = load_sym(handler, md, "pixelCoordsMap", MECHANIC_MODULE_ERROR); 
    if(qpcm) qpcm(ind, c, x, y);
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
void mpi_displayArgs(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data){

  if(mpi_rank == 0){
    if (key->shortName == '?')
      poptPrintHelp(con, stdout, 0);
    else 
      poptPrintUsage(con, stdout, 0);
  }
  con = poptFreeContext(con);
}

void mpi_displayUsage(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data){
    
    if (key->shortName == '?')
      help = 1;
    else
      usage = 1;

}

