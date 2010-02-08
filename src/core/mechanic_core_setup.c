#include "mechanic.h"
#include "mechanic_internals.h"

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
   opts = LRC_textParser(read, sep, comm, cs, ct, numCT);
   if(opts >= 0) printf(" done.\n");
   fclose(read);
  }
  /* If -c is set, but file doesn't exist, abort. */
  else if((read == NULL) && (flag == 1)){
		perror("-> Error opening config file:");
    mechanic_abort(MECHANIC_ERR_SETUP);
	}
  /* We don't insist on having config file present, we just use defaults instead */
  else{
    printf("-> Config file not specified/doesn't exist. Will use defaults.\n");
    opts = 2;
  }
	
  if(opts == 0){
		printf("-> Config file seems to be empty.\n");
	}

  if(opts < 0) mechanic_abort(MECHANIC_ERR_SETUP);
  
  return opts;
}

/* Assign config values, one by one. Final struct contains config values of the run */
int assignConfigValues(int opts, configData* d, LRC_configNamespace* cs, int cflag, int popt){

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
			  if(strcmp(cs[i].options[k].name,"mode") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->mode);
          d->mode = atoi(cs[i].options[k].value); 
        }
    }
    }
    if(strcmp(cs[i].space,"logs") == 0){
		  for(k = 0; k < cs[i].num; k++){
			  if(strcmp(cs[i].options[k].name,"checkpoint") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->checkpoint);
          d->checkpoint = atoi(cs[i].options[k].value); 
        }
      }
    }
	}

 return 0;
}

/* Helper tests */
void poptTestC(char* i, char* j){
    if(strcmp(i, j) != 0) sprintf(i,"%s",j); 
}
void poptTestI(char* i, int j){
    if(atoi(i) != j) sprintf(i,"%d",j);
}
