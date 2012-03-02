/**
 * @file
 */
#include "MBootstrap.h"

/**
 * @function
 * Bootstraps the layer
 *
 * The Mechanic works upon two module layers: the core, and the user-supplied module. All
 * functions that can be specified within the module API are used by the core. This means,
 * that all core setup, core storage etc. is prepared in the same way as in the module.
 *
 * The Bootstrap function is a wrapper for proper module loading:
 * - Loads the specified module
 * - Allocates the memory
 * - Loads the Setup function, if present
 * - Boots LRC API
 */
layer Bootstrap(int node, char* name) {
  layer l;
  query* q = NULL;
  int mstat;
  
  l = Load(node, name);
  
  if (l.handler) {

    /* Setup sane defaults */
    Init(node, &l);

    /* Load module setup */
    q = LoadSym(l.handler, "Setup");
    if (q) { 
      mstat = q(l.setup);
      CheckStatus(mstat);
      l.setup.head = LRC_assignDefaults(l.setup.options);
    }
  } 

  return l;
}

/**
 * @function
 * Wrapper to dlopen()
 */
layer Load(int node, char *name) {
  layer l;
  char *fname = NULL;

  fname = Filename(name);

  Message(MESSAGE_DEBUG, "Loading module '%s'\n", fname);

  l.handler = dlopen(fname, RTLD_NOW|RTLD_GLOBAL);
  if (!l.handler) {
    Message(MESSAGE_ERR, "Cannot load module '%s': %s\n", name, dlerror());
    Error(CORE_ERR_MODULE);
  }
  
  free(fname);
  return l;

}

/**
 * @function
 * Allocates initial memory for the Layer
 */
int Init(int node, layer *l) {
  
  l->setup.options = calloc(CONFIG_OPTIONS*sizeof(LRC_configDefaults), sizeof(LRC_configDefaults));
  if (!l->setup.options) Error(CORE_ERR_MEM);

  l->setup.head = NULL;

  return 0;
}

/**
 * @function
 * Finalizes the layer
 */
void Finalize(int node, layer* l) {
  dlclose(l->handler);
  free(l->setup.options);
  if (l->setup.head) LRC_cleanup(l->setup.head);
}

/**
 * @function
 * Creates module filename
 *
 * @in_group
 * Helpers
 */
char* Filename(char* name) {
  char* fname;
  size_t len, pref, flen;

  pref = strlen(MECHANIC_MODULE_PREFIX);
  len = strlen(name);
  flen = pref + len + strlen(LIBEXT) + 1;

  fname = calloc(flen*sizeof(char*), sizeof(char*));
  if (!fname) Error(CORE_ERR_MEM);

  strncpy(fname, MECHANIC_MODULE_PREFIX, pref);
  fname[pref] = LRC_NULL;

  strncat(fname, name, len);
  fname[pref+len] = LRC_NULL;

  strncat(fname, LIBEXT, strlen(LIBEXT));
  fname[flen] = LRC_NULL;
  
  return fname;
}

