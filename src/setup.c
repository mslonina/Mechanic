#include "mpifarm.h"

/**
 * SETUP TOOLS
 */

/**
 * Default config file parser.
 */
int readDefaultConfig(char* inifile, LRC_configNamespace* cs, LRC_configTypes* ct, int numCT, int flag){

  int i = 0, k = 0, opts = 0, offset = 0;
  char* sep = "=";
  char* comm = "#";

  FILE* read;

  read = fopen(inifile, "r");
  
  /* Default behaviour: try to read default config file. */
  if(read != NULL){
   printf("-> Parsing config file \"%s\"... ", inifile);
   opts = LRC_parseFile(read, sep, comm, cs, ct, numCT);
   if(opts >= 0) printf(" done.\n");
   fclose(read);
  }
  /* If -c is set, but file doesn't exist, abort. */
  else if((read == NULL) && (flag == 1)){
		perror("-> Error opening config file:");
    MPI_Abort(MPI_COMM_WORLD, ERR_SETUP);
	}
  /* We don't insist on having config file present, we just use defaults instead */
  else{
    printf("-> Config file not specified/doesn't exist. Will use defaults.\n");
    opts = 2;
  }
	
  if(opts == 0){
		printf("-> Config file seems to be empty.\n");
	}

  if(opts < 0) MPI_Abort(MPI_COMM_WORLD, ERR_SETUP);
  
  return opts;
}

/* Assign config values, one by one. Final struct contains config values of the run */
void assignConfigValues(int opts, configData* d, LRC_configNamespace* cs, int cflag, int popt){

  int i = 0, k = 0;

  for(i = 0; i < opts; i++){
    if(strcmp(cs[i].space,"default") == 0){
		  for(k = 0; k < cs[i].num; k++){
			  if(strcmp(cs[i].options[k].name,"name") == 0){
          if(popt == 1) poptTestC(cs[i].options[k].value, d->name);
          strcpy(d->name,cs[i].options[k].value);
          strcpy(d->datafile, d->name);
          strcpy(d->datafile, strcat(d->datafile,"-master.h5"));  
        }
			  if(strcmp(cs[i].options[k].name,"module") == 0){
          if(popt == 1) poptTestC(cs[i].options[k].value, d->module);
          strcpy(d->module,cs[i].options[k].value);
        }
			  if(strcmp(cs[i].options[k].name,"xres") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->xres);
          d->xres = atoi(cs[i].options[k].value);  
        }
        if(strcmp(cs[i].options[k].name,"yres") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->yres);
          d->yres = atoi(cs[i].options[k].value); 
        }
			  if(strcmp(cs[i].options[k].name,"method") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->method);
          d->method = atoi(cs[i].options[k].value); 
        }
			  if(strcmp(cs[i].options[k].name,"mrl") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->mrl);
          d->mrl = atoi(cs[i].options[k].value); 
        }
    }
    }
    if(strcmp(cs[i].space,"logs") == 0){
		  for(k = 0; k < cs[i].num; k++){
			  if(strcmp(cs[i].options[k].name,"dump") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->dump);
          d->dump = atoi(cs[i].options[k].value); 
        }
      }
    }
	}

}

/* Helper tests */
void poptTestC(char* i, char* j){
    if(strcmp(i, j) != 0) sprintf(i,"%s",j); 
}
void poptTestI(char* i, int j){
    if(atoi(i) != j) sprintf(i,"%d",j);
}

/**
 * HDF5 config file storage.
 */
void writeConfig(hid_t file_id, int allopts, LRC_configNamespace* cs){
  
  hid_t dset_config, configspace, configmemspace, hdf_config_datatype;
  herr_t hdf_status;

  hsize_t dim[1];
  unsigned rr = 1;
  int j = 0, i = 0, k = 0;

    /* Number of options to write */
    j = 0;
    for(i = 0; i < allopts; i++){
      for(k = 0; k < cs[i].num; k++){
        j++;
      }
    }

    dim[0] = j;

    /* Define simplified struct */
    simpleopts optstohdf[j];
    
    /* Copy config file to our simplified struct */
    j = 0;
      for(i = 0; i < allopts; i++){
        for(k = 0; k < cs[i].num; k++){
          strcpy(optstohdf[j].space,cs[i].space);
          strcpy(optstohdf[j].varname,cs[i].options[k].name);
          strcpy(optstohdf[j].value,cs[i].options[k].value);
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
