/*
 * MECHANIC
 * Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
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
 * http://mechanics.astri.umk.pl/projects/mechanic
 *
 * User guide should be provided with the package or
 * http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
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

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Module file: %s\n", module_file);

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
void mechanic_module_close(mechanic_internals modhand) {
  dlclose(modhand.handler);
  dlclose(modhand.module);
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
void* mechanic_module_sym_lookup(void* modhandler, char* md_name, char* function) {
  char* func;
  char* err;
  void* handler;

  /* Reset dlerror() */
  dlerror();

  /* Prepare symbol name */
  func = mechanic_module_sym_prefix(md_name, function);
  handler = dlsym(modhandler, func);
  err = dlerror();

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Loading function '%s'\n", function);
  free(func);

  if (err == NULL) return handler;

  return NULL;
}

/*
 * Wrapper to dlsym().
 * Handles error messages and abort if necessary.
 *
 * module_master_function
 * module_slave_function
 * todo: module_nodeXX_function
 * module_node_function
 * default_node_function
 * default_function
 */
void* mechanic_load_sym(mechanic_internals modhand, char* function, int type) {

  void *handler;
  char *override;

  /* Master/Slave override */
  if (modhand.node == MECHANIC_MPI_MASTER_NODE) {
    override = mechanic_module_sym_prefix("master", function);
  } else {
    override = mechanic_module_sym_prefix("slave", function);
  }

  handler = mechanic_module_sym_lookup(modhand.handler, modhand.config->module, override);
  free(override);

  if (handler != NULL) return handler;

  /* Node override */
  override = mechanic_module_sym_prefix("node", function);
  handler = mechanic_module_sym_lookup(modhand.handler, modhand.config->module, override);
  free(override);
  if (handler != NULL) return handler;

  /* Module: Function */
  handler = mechanic_module_sym_lookup(modhand.handler, modhand.config->module, function);
  if (handler != NULL) return handler;

  /* Default: Node override */
  override = mechanic_module_sym_prefix("node", function);
  handler = mechanic_module_sym_lookup(modhand.module, MECHANIC_MODULE_DEFAULT, override);
  free(override);
  if (handler != NULL) return handler;

  /* Default: Function */
  handler = mechanic_module_sym_lookup(modhand.module, MECHANIC_MODULE_DEFAULT, function);
  if (handler != NULL) return handler;

  /* Emergency callback: handler not found.
   * This means, that the default module is somehow broken.
   */
  if (handler == NULL) {
    switch (type) {
      case MECHANIC_MODULE_SILENT:
        break;
      case MECHANIC_MODULE_WARN:
        mechanic_message(MECHANIC_MESSAGE_WARN,
            "Module warning: Cannot load function '%s' nor its templates.\n", function);
        break;
      case MECHANIC_MODULE_ERROR:
        mechanic_message(MECHANIC_MESSAGE_ERR,
            "Module error: Cannot load function '%s' nor its templates.\n", function);
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
mechanic_internals mechanic_internals_init(int node, moduleInfo* m, configData* d) {

  mechanic_internals internals;
  char* module_filename;

  module_filename = mechanic_module_filename(d->module);

  /* Load modules */
  internals = mechanic_module_open(module_filename);

  /* Allocate schema */
  m->schema = calloc(sizeof(mechanicSchema) * m->schemasize, sizeof(mechanicSchema));

  /* Fill the rest of the structure */
  internals.node = node;
  internals.config = d;
  internals.info = m;

  free(module_filename);

  return internals;
}

/*
 * Finalize any internal data
 */
void mechanic_internals_close(mechanic_internals modhand) {
  mechanic_module_close(modhand);
  free(modhand.info->schema);
}
