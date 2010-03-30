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

/* SETUP TOOLS */

/* Default config file parser */
int readDefaultConfig(char* inifile, int flag){

  int opts = 0;
  char* sep = "=";
  char* comm = "#";

  FILE* read;

  read = fopen(inifile, "r");
  
  /* Default behaviour: try to read default config file. */
  if(read != NULL){
   mechanic_message(MECHANIC_MESSAGE_INFO,"Parsing config file \"%s\"... ", inifile);
   opts = LRC_ASCIIParser(read, sep, comm);
   if(opts >= 0) mechanic_message(MECHANIC_MESSAGE_CONT," done.\n");
   fclose(read);
  }
  /* If -c is set, but file doesn't exist, abort. */
  else if((read == NULL) && (flag == 1)) mechanic_error(MECHANIC_ERR_SETUP);
  /* We don't insist on having config file present, we just use defaults instead */
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

/* Assign config values, one by one. Final struct contains config values of the run */ 
int assignConfigValues(configData* d){

  char* n = NULL; char* tf = NULL; char* m = NULL;
  size_t nlen, flen, fmlen, mlen;
  char* tempaddr = NULL;

  /* Prepare the name of the problem */
  n = LRC_getOptionValue("default", "name");
  nlen = strlen(n);
  
  if(d->name == NULL){
    d->name = malloc(nlen + sizeof(char*));
    if(d->name == NULL) {
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }else{
    tempaddr = realloc(d->name, nlen + sizeof(char*));
    if(d->name == NULL){
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }
  
  strncpy(d->name, n, nlen);
  d->name[nlen] = LRC_NULL;
  
  /* Prepare the file name */
  fmlen = strlen(MECHANIC_MASTER_SUFFIX_DEFAULT) + sizeof(char*);
  flen = nlen + fmlen + 2*sizeof(char*);

  tf = malloc(flen + sizeof(char*));
  if(tf == NULL) {
    mechanic_error(MECHANIC_ERR_MEM);
  }

  strncpy(tf, d->name, nlen);
  tf[nlen] = LRC_NULL;

  strncat(tf, MECHANIC_MASTER_SUFFIX_DEFAULT, fmlen);
  tf[flen] = LRC_NULL;

  if(d->datafile == NULL){
    d->datafile = malloc(flen + sizeof(char*));
    if(d->datafile == NULL){
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }else{
    tempaddr = realloc(d->datafile, flen + sizeof(char*));
    if(d->datafile == NULL){
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }
  
  strncpy(d->datafile, tf, flen);
  d->datafile[flen] = LRC_NULL;

  /* Prepare the module name */
  m = LRC_getOptionValue("default", "module");
  mlen = strlen(m);
  
  if(d->module == NULL){
    d->module = malloc(nlen + sizeof(char*));
    if(d->module == NULL){
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }else{
    tempaddr = realloc(d->module, nlen + sizeof(char*));
    if(d->module == NULL){
      mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  strncpy(d->module, m, mlen);
  d->module[mlen] = LRC_NULL;

  /* Other options */
	d->xres = LRC_option2int("default", "xres");
	d->yres = LRC_option2int("default", "yres");
	d->method = LRC_option2int("default", "method");
	d->checkpoint = LRC_option2int("logs", "checkpoint");
	d->restartmode = 0;
	d->mode = LRC_option2int("default", "mode");

  /* Free resources */
  free(tf);

 return 0;
}

