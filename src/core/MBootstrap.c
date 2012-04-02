/**
 * @file
 * The Bootstrap stage.
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
module Bootstrap(int node, int mpi_size, int argc, char **argv, char *name, module *f) {
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

    /* Popt defaults, global options */
    m.popt->popt[0] = (struct poptOption) 
      {"module", 'p', POPT_ARG_STRING, &m.popt->string_args[0], 0, "Module name", NULL};
    m.popt->popt[1] = (struct poptOption) 
      {"config", 'c', POPT_ARG_STRING, &m.popt->string_args[1], 0, "Config filename", NULL};
    m.popt->popt[2] = (struct poptOption)
      {"help", '?', POPT_ARG_VAL, &m.popt->int_args[2], 1, "Show this help message", NULL}; 
    m.popt->popt[3] = (struct poptOption)
      {"usage", '\0', POPT_ARG_VAL, &m.popt->int_args[3], 1, "Display brief message", NULL}; 
    m.popt->popt[4] = (struct poptOption) POPT_TABLEEND;

    /* Get the global Popt context */
    m.popt->poptcontext = poptGetContext(NULL, argc, (const char **) argv, m.popt->popt, 0);
    poptGetNextOpt(m.popt->poptcontext);

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

  fname = Name(MECHANIC_MODULE_PREFIX, name, "", LIBEXT);

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
  
  m->layer.setup.popt = calloc(sizeof(popt), sizeof(popt));
  if (!m->layer.setup.popt) Error(CORE_ERR_MEM);

  m->layer.setup.popt->popt = calloc(3*opts*sizeof(struct poptOption), sizeof(struct poptOption));
  if (!m->layer.setup.popt->popt) Error(CORE_ERR_MEM);
  
  m->layer.setup.popt->string_args = calloc(3*opts*sizeof(char), sizeof(char));
  if (!m->layer.setup.popt->string_args) Error(CORE_ERR_MEM);
  
  m->layer.setup.popt->int_args = calloc(3*opts*sizeof(int), sizeof(int));
  if (!m->layer.setup.popt->int_args) Error(CORE_ERR_MEM);

  m->layer.setup.popt->double_args = calloc(3*opts*sizeof(double), sizeof(double));
  if (!m->layer.setup.popt->double_args) Error(CORE_ERR_MEM);

  m->popt = calloc(sizeof(popt), sizeof(popt));
  if (!m->popt) Error(CORE_ERR_MEM);
  
  m->popt->popt = calloc(opts*sizeof(struct poptOption), sizeof(struct poptOption));
  if (!m->popt->popt) Error(CORE_ERR_MEM);
  
  m->popt->string_args = calloc(opts*sizeof(char), sizeof(char));
  if (!m->popt->string_args) Error(CORE_ERR_MEM);
  
  m->popt->int_args = calloc(opts*sizeof(int), sizeof(int));
  if (!m->popt->int_args) Error(CORE_ERR_MEM);

  m->popt->double_args = calloc(opts*sizeof(double), sizeof(double));
  if (!m->popt->double_args) Error(CORE_ERR_MEM);

  m->layer.setup.head = NULL;

  return mstat;
}

/**
 * @function
 * Initializes the Setup
 *
 * This function calls the Setup() from the module to initialize the LRC default option
 * structure. It merges with the fallback structure, so that all setup is available in one
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
    } 
  }
  
  m->layer.setup.head = NULL;
  m->layer.setup.popt->poptcontext = NULL;

  return mstat;
}

/**
 * @function
 * Finalizes the layer
 */
void FinalizeLayer(layer *l) {
  if (l->handler) dlclose(l->handler);
  if (l->setup.options) free(l->setup.options);
  if (l->setup.popt->popt) free(l->setup.popt->popt);
  if (l->setup.popt->string_args) free(l->setup.popt->string_args);
  if (l->setup.popt->int_args) free(l->setup.popt->int_args);
  if (l->setup.popt->double_args) free(l->setup.popt->double_args);
  if (l->setup.popt->poptcontext) poptFreeContext(l->setup.popt->poptcontext);
  if (l->setup.popt) free(l->setup.popt);
  if (l->setup.head) LRC_cleanup(l->setup.head);
}

/**
 * @function
 * Finalizes the module
 */
void ModuleFinalize(module* m) {
  FinalizeLayer(&m->layer);
  if (m->popt->popt) free(m->popt->popt);
  if (m->popt->string_args) free(m->popt->string_args);
  if (m->popt->int_args) free(m->popt->int_args);
  if (m->popt->double_args) free(m->popt->double_args);
  if (m->popt->poptcontext) poptFreeContext(m->popt->poptcontext);
  if (m->popt) free(m->popt);
}

