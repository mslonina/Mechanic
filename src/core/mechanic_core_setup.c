/*
 * MECHANIC Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 * 
 * This file is part of MECHANIC code. 
 *
 * MECHANIC was created to help solving many numerical problems by providing tools
 * for improving scalability and functionality of the code. MECHANIC was released 
 * in belief it will be useful. If you are going to use this code, or its parts,
 * please consider referring to the authors either by the website or the user guide 
 * reference.
 *
 * http://mechanics.astri.umk.pl/projects/mechanic
 *
 * User guide should be provided with the package or 
 * http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
 *
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided 
 * that the following conditions are met:
 * 
 *  - Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *  - Neither the name of the Nicolaus Copernicus University nor the names of 
 *    its contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#include "mechanic.h"
#include "mechanic_internals.h"

/*
 * SETUP TOOLS
 */

/*
 * Default config file parser
 */
int readDefaultConfig(char* inifile, LRC_configNamespace* cs, LRC_configTypes* ct, int numCT, int flag){

  int opts = 0;
  char* sep = "=";
  char* comm = "#";

  FILE* read;

  read = fopen(inifile, "r");
  
  // Default behaviour: try to read default config file. 
  if(read != NULL){
   mechanic_message(MECHANIC_MESSAGE_INFO,"Parsing config file \"%s\"... ", inifile);
   opts = LRC_textParser(read, sep, comm, cs, ct, numCT);
   if(opts >= 0) mechanic_message(MECHANIC_MESSAGE_CONT," done.\n");
   fclose(read);
  }
  // If -c is set, but file doesn't exist, abort. 
  else if((read == NULL) && (flag == 1)) mechanic_error(MECHANIC_ERR_SETUP);
  // We don't insist on having config file present, we just use defaults instead 
  else{
    mechanic_message(MECHANIC_MESSAGE_WARN, "Config file not specified/doesn't exist. Will use defaults.\n");
    opts = 2;
  }
	
  if(opts == 0){
		mechanic_message(MECHANIC_MESSAGE_WARN, "Config file seems to be empty.\n");
	}

  if(opts < 0) mechanic_error(MECHANIC_ERR_SETUP);
  
  return opts;
}

// Assign config values, one by one. Final struct contains config values of the run 
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
			  if(strcmp(cs[i].options[k].name,"checkpoint_num") == 0){
          if(popt == 1) poptTestI(cs[i].options[k].value, d->checkpoint_num);
          d->checkpoint_num = atoi(cs[i].options[k].value); 
        }
      }
    }
	}

 return 0;
}

// Helper tests 
void poptTestC(char* i, char* j){
    if(strcmp(i, j) != 0) sprintf(i,"%s",j); 
}
void poptTestI(char* i, int j){
    if(atoi(i) != j) sprintf(i,"%d",j);
}
