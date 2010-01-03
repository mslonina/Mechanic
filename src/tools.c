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
int* map2d(int c, void* module, configData* d){
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
    qpcm = load_sym(module, "userdefined_pixelCoordsMap", MODULE_ERROR); 
    if(qpcm) qpcm(ind, c, x, y);
   }

   return ind;
}

/**
 * Wrapper to load_sym().
 * Handles error messages and abort if necessary.
 */
void* load_sym(void* module, char* function, int type){
 
  void* handler;
  char* err;

  dlerror();
  handler = dlsym(module, function);
  if((err = dlerror()) != NULL){
    switch (type){
      case MODULE_SILENT:
        break;
      case MODULE_WARN:
        printf("-> Module warning: Cannot load function '%s': %s\n", function, err); 
        break;
      case MODULE_ERROR:
        printf("-> Module error: Cannot load function '%s': %s\n", function, err); 
        break;
      default:
        break;
    }
    if(type == MODULE_ERROR){
      MPI_Abort(MPI_COMM_WORLD, ERR_MODULE);
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

/**
 * HDF5 data storage
 */
void H5writeMaster(hid_t dset, hid_t memspace, hid_t space, configData* d, masterData* rawdata){
  
  MY_DATATYPE rdata[d->mrl][1];
  hsize_t co[2], off[2];
  herr_t hdf_status;
  int j = 0;
      
  co[0] = 1;
  co[1] = d->mrl;

  off[0] = rawdata->coords[2];
  off[1] = 0;
  
  for (j = 0; j < d->mrl; j++){
    rdata[j][0] = rawdata->res[j];
  }
      
  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, memspace, space, H5P_DEFAULT, rdata);

}

void H5writeBoard(hid_t dset, hid_t memspace, hid_t space, masterData *rawdata){
  
  int rdata[1][1];
  hsize_t co[2], off[2];
  herr_t hdf_status;
      
  co[0] = 1;
  co[1] = 1;

  off[0] = rawdata->coords[0];
  off[1] = rawdata->coords[1];
 
  rdata[0][0] = 1;
      
  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_INT, memspace, space, H5P_DEFAULT, rdata);

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

