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
module Bootstrap(int node, int mpi_size, char *name, module *f) {
  module m;
  
  m = ModuleLoad(name);

  m.node = node;
  m.mpi_size = mpi_size;

  /* Fallback module */
  if (f->layer.handler) {
    m.fallback.handler = f->layer.handler;
    m.fallback.init = f->layer.init;
    m.fallback.setup = f->layer.setup;
  }

  if (m.layer.handler) {

    /* Initialize the module */
    ModuleInit(&m);

    /* Load module Setup */
    ModuleSetup(&m);

  } 

  return m;
}

/**
 * @function
 * Wrapper to dlopen()
 */
module ModuleLoad(char *name) {
  module m;
  char *fname = NULL;

  fname = Filename(MECHANIC_MODULE_PREFIX, name, "", LIBEXT);

  Message(MESSAGE_DEBUG, "Loading module '%s'\n", fname);

  m.layer.handler = dlopen(fname, RTLD_NOW|RTLD_GLOBAL);
  if (!m.layer.handler) {
    Message(MESSAGE_ERR, "Cannot load module '%s': %s\n", name, dlerror());
    Error(CORE_ERR_MODULE);
  }

  m.fallback.handler = NULL;
  
  free(fname);
  return m;

}

/**
 * @function
 * Allocates initial memory for the Layer
 *
 * - Load fallback layer function
 * - If the layer function exists, overwrite fallback
 */
int ModuleInit(module *m) {
  query *q;
  int opts;
  int mstat;

  /* Load fallback layer, at least core module must implement this */
  if (m->fallback.handler) {
    m->layer.init = m->fallback.init;
  }

  q = LoadSym(m, "Init", NO_FALLBACK);
  if (q) mstat = q(&m->layer.init);
  CheckStatus(mstat);
   
  opts = m->layer.init.options;
  if (m->fallback.handler) opts = opts + m->fallback.init.options;
  
  m->layer.setup.options = calloc(opts*sizeof(LRC_configDefaults), sizeof(LRC_configDefaults));
  if (!m->layer.setup.options) Error(CORE_ERR_MEM);
  m->layer.setup.popt = calloc((opts+2)*sizeof(struct poptOption), sizeof(struct poptOption));
  if (!m->layer.setup.popt) Error(CORE_ERR_MEM);

  m->layer.setup.head = NULL;

  return mstat;
}

/**
 * @function
 * Initializes the Setup
 *
 * This function calls the Setup() from the module to initialize the LRC default option
 * structure. It merges the fallback structure, so that all setup is available in one
 * layer (and only one config file can be used, with i.e. core setup included, but not
 * necessary).
 */
int ModuleSetup(module *m) {
  query *q;
  int mstat;

  q = LoadSym(m, "Setup", NO_FALLBACK);
  if (q) {
    mstat = q(&m->layer.setup);
    CheckStatus(mstat);

    /**
     * Note:
     * Since we merge core layer to module layer, the user cannot change the core defaults
     */
    if (m->fallback.handler) {
      mstat = LRC_mergeDefaults(m->layer.setup.options, m->fallback.setup.options);
      //mstat = PoptMergeOptions(m, m->layer.setup.popt, m->fallback.setup.popt);
    } 

    m->layer.setup.head = LRC_assignDefaults(m->layer.setup.options);
  }

  return mstat;
}

/**
 * @function
 * Finalizes the layer
 */
void FinalizeLayer(layer *l) {
  dlclose(l->handler);
  free(l->setup.options);
  free(l->setup.popt);
  //poptFreeContext(l->setup.poptcontext);
  if (l->setup.head) LRC_cleanup(l->setup.head);
}

/**
 * @function
 * Finalizes the module
 */
void ModuleFinalize(module* m) {
  FinalizeLayer(&m->layer);
}

