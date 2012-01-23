/*
 * MECHANIC
 *
 * Copyright (c) 2010-2011, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 *
 * This file is part of MECHANIC code.
 *
 * MECHANIC was created to help solving many numerical problems by providing
 * tools for improving scalability and functionality of the code. MECHANIC was
 * released in belief it will be useful. If you are going to use this code, or
 * its parts, please consider referring to the authors either by the website
 * or the user guide reference.
 *
 * http://git.astri.umk.pl/projects/mechanic
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Nicolaus Copernicus University nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* [INTERNALS] */

/* [/INTERNALS] */

#include "mechanic.h"
#include "mechanic_internals.h"

/*
 * Returns filename of the module
 */
char* mechanic_module_filename(char* name){

  char* module_file;
  size_t module_len, module_pref, module_file_len;

  module_pref = strlen(MECHANIC_MODULE_PREFIX);
  module_len = strlen(name);
  module_file_len = module_pref + module_len + LIB_ESZ + 1;

  module_file = calloc((module_file_len)*sizeof(char*), sizeof(char*));
  if (module_file == NULL) mechanic_error(MECHANIC_ERR_MEM);

  strncpy(module_file, MECHANIC_MODULE_PREFIX, module_pref);
  module_file[module_pref] = LRC_NULL;

  strncat(module_file, name, module_len);
  module_file[module_pref+module_len] = LRC_NULL;

  strncat(module_file, LIB_EXT, LIB_ESZ);
  module_file[module_file_len] = LRC_NULL;

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "TaskInfo file: %s\n", module_file);

  return module_file;
}

/*
 * Wrapper to dlopen()
 *
 */
mechanic_internals mechanic_module_open(char* modulename) {

  mechanic_internals modhand;
  char* defaultname;

  defaultname = mechanic_module_filename(MECHANIC_MODULE_DEFAULT);

  modhand.module = dlopen(defaultname, RTLD_NOW|RTLD_GLOBAL);
  if (!modhand.module) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Cannot load module '%s': %s\n", MECHANIC_MODULE_DEFAULT, dlerror());
    mechanic_error(MECHANIC_ERR_MODULE);
  }

  modhand.handler = dlopen(modulename, RTLD_NOW|RTLD_GLOBAL);
  if (!modhand.handler) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Cannot load module '%s': %s\n", modulename, dlerror());
    mechanic_error(MECHANIC_ERR_MODULE);
  }

  free(defaultname);

  return modhand;
}

/*
 * Wrapper to dlclose()
 *
 */
void mechanic_module_close(mechanic_internals* modhand) {
  dlclose(modhand->handler);
  dlclose(modhand->module);
}

/*
 * Creates function name
 */
char* mechanic_module_sym_prefix(char* md_name, char* function) {

  char* func;
  size_t fl, mn, lenl;

  mn = strlen(md_name);
  fl = strlen(function);
  lenl = mn + fl + 3;

  func = calloc(lenl * sizeof(char*), sizeof(char*));
  if (func == NULL) mechanic_error(MECHANIC_ERR_MEM);

  strncpy(func, md_name, mn);
  func[mn] = LRC_NULL;

  strncat(func, "_", 1);
  func[mn+1] = LRC_NULL;

  strncat(func, function, fl);
  func[lenl] = LRC_NULL;

  return func;
}

/*
 * Helper for mechanic_load_sym()
 */
module_query_int_f mechanic_module_sym_lookup(void* modhandler, char* md_name, char* function) {
  char* func;
  char* err;
  module_query_int_f handler;

  /* Reset dlerror() */
  dlerror();

  /* Prepare symbol name */
  func = mechanic_module_sym_prefix(md_name, function);
  handler = (module_query_int_f) dlsym(modhandler, func);
  err = dlerror();

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Querying function '%s'\n", function);
  free(func);

  if (err == NULL) {
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Loading function '%s'\n", function);
    return handler;
  }

  return NULL;
}

/*
 * Wrapper to dlsym().
 * Handles error messages and abort if necessary.
 *
 * module_master_function
 * module_worker_function
 * todo: module_nodeXX_function
 * module_node_function
 * default_node_function
 * default_function
 */
module_query_int_f mechanic_load_sym(mechanic_internals *modhand, char* function, int type, int tp) {

  module_query_int_f handler;
  char *override;

  if (tp == MECHANIC_TEMPLATE) {
    /* Master/Slave override */
    if (modhand->node == MECHANIC_MPI_MASTER_NODE) {
      override = mechanic_module_sym_prefix("master", function);
    } else {
      override = mechanic_module_sym_prefix("worker", function);
    }

    handler = mechanic_module_sym_lookup(modhand->handler, modhand->config->module, override);
    free(override);
    if (handler != NULL) return handler;

    /* Node override */
    override = mechanic_module_sym_prefix("node", function);
    handler = mechanic_module_sym_lookup(modhand->handler, modhand->config->module, override);
    free(override);
    if (handler != NULL) return handler;

    /* TaskInfo: Function */
    handler = mechanic_module_sym_lookup(modhand->handler, modhand->config->module, function);
    if (handler != NULL) return handler;

    /* Default: Node override */
    override = mechanic_module_sym_prefix("node", function);
    handler = mechanic_module_sym_lookup(modhand->module, MECHANIC_MODULE_DEFAULT, override);
    free(override);
    if (handler != NULL) return handler;

    /* Default: Function */
    handler = mechanic_module_sym_lookup(modhand->module, MECHANIC_MODULE_DEFAULT, function);
    if (handler != NULL) return handler;

  } else {
    /* TaskInfo: Function */
    handler = mechanic_module_sym_lookup(modhand->handler, modhand->config->module, function);
    if (handler != NULL) return handler;

    /* Default: Function */
    handler = mechanic_module_sym_lookup(modhand->module, MECHANIC_MODULE_DEFAULT, function);
    if (handler != NULL) return handler;
  }

  /* Emergency callback: handler not found.
   * This means, that the default module is somehow broken.
   */
  if (handler == NULL) {
    switch (type) {
      case MECHANIC_MODULE_SILENT:
        break;
      case MECHANIC_MODULE_WARN:
        mechanic_message(MECHANIC_MESSAGE_WARN,
            "TaskInfo warning: Cannot load function '%s' nor its templates.\n", function);
        break;
      case MECHANIC_MODULE_ERROR:
        mechanic_message(MECHANIC_MESSAGE_ERR,
            "TaskInfo error: Cannot load function '%s' nor its templates.\n", function);
        break;
      default:
        break;
    }

    if (type == MECHANIC_MODULE_ERROR) mechanic_error(MECHANIC_ERR_MODULE);
  }

  /* This should never happen */
  return NULL;

}

/*
 * Initialize internal data structure
 */
mechanic_internals mechanic_internals_init(int mpi_size, int node, TaskInfo* m, TaskConfig* d) {

  mechanic_internals internals;
  char* module_filename;

  module_filename = mechanic_module_filename(d->module);

  /* Load modules */
  internals = mechanic_module_open(module_filename);

  /* Fill the rest of the structure */
  internals.node = node;
  internals.mpi_size = mpi_size;
  internals.config = d;
  internals.info = m;
  
  free(module_filename);

  return internals;
}

int prepare_ice(mechanic_internals *internals) {
  int mstat = 0;
  char* buf;
  size_t len, flen;

  len = internals->config->name_len;
  flen = strlen(internals->config->name) + 1;
  buf = calloc(flen + 4 + 2*sizeof(char*), sizeof(char*));
  if (!buf) mechanic_error(MECHANIC_ERR_MEM);

  strncpy(buf, internals->config->name, len);
  buf[len] = LRC_NULL;

  strncat(buf, ".ice", 4);
  buf[len+4] = LRC_NULL;

  strncpy(internals->ice, buf, len+4);
  internals->ice[len+4] = LRC_NULL;

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "ICE FILE IS: %s\n", internals->ice);

  free(buf);
  return mstat;
}

/*
 * Allocate memory for schema
 */
void mechanic_internals_schema_init(int node, TaskInfo* m, mechanic_internals* internals) {

  /* Allocate schema */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Schemasize is %d\n", m->schemasize);
  if (m->schemasize > MECHANIC_MAX_HDF_RANK) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "HDF5 MAX RANK exceeded.\n");
    mechanic_abort(MECHANIC_ERR_SETUP);
  }

  m->schema = calloc(sizeof(mechanicSchema) * m->schemasize, sizeof(mechanicSchema));
  if (!m->schema) {
    mechanic_abort(MECHANIC_ERR_MEM);
  }
  
  /* Allocate module setup */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Setupsize is %d\n", m->options);
  m->mconfig = realloc(m->mconfig, sizeof(LRC_configDefaults) * (m->options + 1));
  if (!m->mconfig) {
    mechanic_abort(MECHANIC_ERR_MEM);
  }

  internals->info = m;

}

/*
 * Finalize any internal data
 */
void mechanic_internals_close(mechanic_internals* modhand) {
  mechanic_module_close(modhand);
  free(modhand->info->schema);
  free(modhand->info->mconfig);
}

/*
 * Helper for MPI LRC derived type
 */
LRC_MPIStruct* allocateLRCMPIStruct(int options) {
  LRC_MPIStruct* cc;

  cc = (LRC_MPIStruct*)malloc(sizeof(LRC_MPIStruct) * options);
  if (!cc) {
    mechanic_abort(MECHANIC_ERR_MEM);
  }

  return cc;
}

/*
 * Converts LRC linked list to MPI structure
 */
int LRC2MPI(LRC_MPIStruct* cc, LRC_configNamespace* head) {

  int mstat = 0, i = 0;
  size_t clen;
  LRC_configOptions* currentOP = NULL;
  LRC_configOptions* nextOP = NULL;
  LRC_configNamespace* nextNM = NULL;
  LRC_configNamespace* current = NULL;

  if (head) {
    current = head;

    do { 
      if (current) {
        nextNM = current->next;
        currentOP = current->options;
        do { 
          if (currentOP) {
            clen = strlen(current->space) + 1; 
            strncpy(cc[i].space, current->space, clen);
            clen = strlen(currentOP->name) + 1; 
            strncpy(cc[i].name, currentOP->name, clen);
            clen = strlen(currentOP->value) + 1; 
            strncpy(cc[i].value, currentOP->value, clen);
            cc[i].type = currentOP->type;
            mechanic_message(MECHANIC_MESSAGE_DEBUG, "[%d] %s %s %s %d\n", i, cc[i].space, cc[i].name, cc[i].value, cc[i].type);
            i++; 
            nextOP = currentOP->next;
            currentOP = nextOP;
          }    
        } while(currentOP);
        current=nextNM;
      }    
    } while(current);
  }

  return mstat;
}
