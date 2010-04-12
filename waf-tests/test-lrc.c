#include <stdio.h>
#include <stdlib.h>
#include "libreadconfig_hdf5.h"
#include "hdf5.h"

int main(int argc, char* argv[]){

  hid_t file;
  
  LRC_configDefaults ct[] = {
    {"default", "inidata", "test.dat", LRC_STRING},
    {"logs", "dump", "100", LRC_INT},
    {"farm", "yres", "444", LRC_INT},
    LRC_OPTIONS_END
  };
	
  LRC_assignDefaults(ct); 
  
	file = H5Fcreate("lrc-hdf-test.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  LRC_HDF5Writer(file);
  H5Fclose(file);

  remove("lrc-hdf-test.h5");

	LRC_cleanup();
	
  return 0;
}
