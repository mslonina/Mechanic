/**
 * We are now on the eve of the second transit of a pair, after which there will be no
 * other till the twenty-first century of our era has dawned upon the earth, and the June
 * flowers are blooming in 2004. When the last transit season occurred the intellectual
 * world was awakening from the slumber of ages, and that wondrous scientific activity
 * which has led to our present advanced knowledge was just beginning. What will be the
 * state of science when the next transit season arrives God only knows. Not even our
 * children's children will live to take part in the astronomy of that day. As for
 * ourselves, we have to do with the present...
 *
 * William Harkness, The Transit of Venus
 *
 * I think the astronomers of the first years of the twenty first century, looking back
 * over the long transit-less period which will then have passed, will understand the
 * anxiety of astronomers in our own time to utilise to the full whatever opportunities
 * the coming transits may afford...;and I venture to hope...they will not be disposed to
 * judge over harshly what some in our own day may have regarded as an excess of zeal.
 *
 * Richard Proctor, Transits of Venus, A Popular Account, 1875
 */

/**
 * @file
 * The Bootstrap stage
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
    ModuleSetup(&m, argc, argv);

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

  m.layer.handler = NULL;

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
  query *q = NULL;
  int opts = 0;
  int mstat = 0;

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

  m->layer.setup.head = NULL;
  m->layer.setup.popt->poptcontext = NULL;
  m->filename = NULL;

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
int ModuleSetup(module *m, int argc, char **argv) {
  query *q = NULL;
  int mstat = 0;

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

  m->layer.setup.head = LRC_assignDefaults(m->layer.setup.options);

  /* Popt options */
  mstat = PoptOptions(m, &m->layer.setup);
  m->layer.setup.popt->poptcontext = poptGetContext(NULL, argc, (const char **) argv, m->layer.setup.popt->popt, 0);
  poptGetNextOpt(m->layer.setup.popt->poptcontext);

  LRCUpdate(&m->layer.setup);

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
  if (m->layer.handler) FinalizeLayer(&m->layer);
  if (m->filename) free(m->filename);
}

