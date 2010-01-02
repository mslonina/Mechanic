#include "mpifarm.h"

/**
 * SETUP TOOLS
 */

/**
 * Default config file parser.
 */
int readDefaultConfig(char* inifile, configData *d){

  int i = 0, k = 0, opts = 0, offset = 0;
  char* sep = "=";
  char* comm = "#";

  //printf("start parse...");
  opts = parseConfigFile(inifile, sep, comm);
  //printf(" parsed %d opts\n",opts);
  //printAll(opts);
	for(i = 0; i < opts; i++){
  //  printf("loop %d\n", i);
    if(strcmp(configSpace[i].space,"default") == 0){
		  for(k = 0; k < configSpace[i].num; k++){
			  if(strcmp(configSpace[i].options[k].name,"name") == 0){
          strcpy(d->name,configSpace[i].options[k].value);
          strcpy(d->datafile, strcat(configSpace[i].options[k].value,"-master.h5"));  
        }
			  if(strcmp(configSpace[i].options[k].name,"module") == 0){
          strcpy(d->module,configSpace[i].options[k].value);
        }
			  if(strcmp(configSpace[i].options[k].name,"xres") == 0) d->xres = atoi(configSpace[i].options[k].value);  
        if(strcmp(configSpace[i].options[k].name,"yres") == 0) d->yres = atoi(configSpace[i].options[k].value); 
			  if(strcmp(configSpace[i].options[k].name,"method") == 0) d->method = atoi(configSpace[i].options[k].value); 
			  if(strcmp(configSpace[i].options[k].name,"mrl") == 0) d->mrl = atoi(configSpace[i].options[k].value); 
    }
    }
    if(strcmp(configSpace[i].space,"logs") == 0){
		  for(k = 0; k < configSpace[i].num; k++){
			  if(strcmp(configSpace[i].options[k].name,"dump") == 0) d->dump = atoi(configSpace[i].options[k].value); 
      }
    }
	}
  //printf("after loop\n");

  return opts;
}

/**
 * HDF5 config file storage.
 */
void writeConfig(hid_t file_id, int allopts){
  hid_t dset_config, configspace, configmemspace, hdf_config_datatype;
  herr_t hdf_status;

  hsize_t dim[1];
  unsigned rr = 1;
  int j = 0, i = 0, k = 0;

    /* Number of options to write */
    j = 0;
    for(i = 0; i < allopts; i++){
      for(k = 0; k < configSpace[i].num; k++){
        j++;
      }
    }

    dim[0] = j;

    /* Define simplified struct */
    simpleopts optstohdf[j];
    
    /* Copy config file to our simplified struct */
    j = 0;
      for(i = 0; i < allopts; i++){
        for(k = 0; k < configSpace[i].num; k++){
          strcpy(optstohdf[j].space,configSpace[i].space);
          strcpy(optstohdf[j].varname,configSpace[i].options[k].name);
          strcpy(optstohdf[j].value,configSpace[i].options[k].value);
          j++;
        }
      }

    /* Config dataspace */
    configspace = H5Screate_simple(rr, dim, NULL);

    /* Create compound data type for handling config struct */
    hid_t hdf_optsarr_dt = H5Tcreate(H5T_COMPOUND, sizeof(simpleopts));
    
      hid_t space_dt = H5Tcopy(H5T_C_S1);
      H5Tset_size(space_dt, MAX_NAME_LENGTH);
    
      hid_t varname_dt = H5Tcopy(H5T_C_S1);
      H5Tset_size(varname_dt, MAX_NAME_LENGTH);
    
      hid_t value_dt = H5Tcopy(H5T_C_S1);
      H5Tset_size(value_dt, MAX_NAME_LENGTH);

    H5Tinsert(hdf_optsarr_dt, "Namespace", HOFFSET(simpleopts, space), space_dt);
    H5Tinsert(hdf_optsarr_dt, "Variable", HOFFSET(simpleopts, varname), varname_dt);
    H5Tinsert(hdf_optsarr_dt, "Value", HOFFSET(simpleopts, value), value_dt);

    dset_config = H5Dcreate(file_id, DATASETCONFIG, hdf_optsarr_dt, configspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    configmemspace = H5Screate_simple(rr, dim, NULL);

    hdf_status = H5Dwrite(dset_config, hdf_optsarr_dt, H5S_ALL, H5S_ALL, H5P_DEFAULT, optstohdf);

    H5Tclose(hdf_optsarr_dt);
    H5Dclose(dset_config);
    H5Sclose(configspace);
    H5Sclose(configmemspace);

}
