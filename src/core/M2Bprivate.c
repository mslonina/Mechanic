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
 * The bootstrap stage
 */
#include "M2Bprivate.h"

/**
 * @brief Bootstraps the layer
 *
 * The Mechanic works upon two module layers: the core, and the user-supplied module. All
 * functions that can be specified within the module API are used by the core. This means,
 * that all core setup, core storage etc. is prepared in the same way as in the module.
 *
 * The Bootstrap function is a wrapper for proper module loading:
 * - Loads the specified module
 * - Allocates the memory
 * - Loads the Setup function, if present
 * - Boots Config API
 *
 * @param node The current node ID
 * @param mpi_size The MPI_COMM_WORLD size
 * @param argc The command line argc table
 * @param argv The command line argv table
 * @param name The name of the module to load
 * @param f The fallback module pointer
 *
 * @return The module pointer, NULL otherwise
 */
module* Bootstrap(int node, int mpi_size, int argc, char **argv, char *name, module *f) {
  module *m = NULL;

  m = ModuleLoad(name);

  if (m) {

    m->node = node;
    m->mpi_size = mpi_size;

    /* Fallback module */
    if (f) {
      if (f->layer) {
        if (f->layer->handler) {
          m->fallback->handler = f->layer->handler;
          m->fallback->mode_handler = f->layer->mode_handler;
          m->fallback->init = f->layer->init;
          m->fallback->setup = f->layer->setup;
        }
      }
    }

    if (m->layer) {
      if (m->layer->handler) {

      /* Initialize the module */
      ModuleInit(m);

      /* Load module Setup */
      ModuleSetup(m, argc, argv);
      }
    }
  }

  return m;
}

/**
 * @brief Loads the runtime mode library
 *
 * @param name The of the runtime mode to load
 *
 * @return The module handler, NULL otherwise
 */
void* RuntimeModeLoad(char *name) {
  char *fname = NULL;
  void *handler = NULL;
  fname = Name(MECHANIC_MODE_PREFIX, name, "", LIBEXT);

  handler = dlopen(fname, RTLD_NOW|RTLD_GLOBAL);
  if (!handler) {
    Message(MESSAGE_ERR, "Cannot load runtime mode '%s': %s\n", name, dlerror());
    Error(CORE_ERR_MODULE);
  }

  free(fname);
  return handler;
}

/**
 * @brief Initialize the runtime mode
 *
 * @param handler The runtime mode handler
 * @param m The module pointer
 *
 * @return SUCCESS on success, error code otherwise
 */
int RuntimeModeInit(module *m) {
  query *q = NULL;
  char *err;
  int mstat = SUCCESS;
  init i;

  q = (query*) dlsym(m->layer->mode_handler, "Init");
  err = dlerror();
  if (err == NULL) {
    if (q) mstat = q(&i);
    CheckStatus(mstat);

    if (i.min_cpu_required > m->layer->init->min_cpu_required) {
      m->layer->init->min_cpu_required = i.min_cpu_required;
    }
  }

  return mstat;
}

/**
 * @brief Load the module layer
 *
 * @return The layer pointer on success, NULL otherwise
 */
layer* LayerLoad(void) {
  layer *l = NULL;

  l = calloc(1, sizeof(layer));
  if (!l) {
    Message(MESSAGE_ERR, "Cannot allocate module layer\n");
    Error(CORE_ERR_MEM);
  }

  l->init = NULL;
  l->setup = NULL;

  l->init = calloc(1, sizeof(init));
  if (!l->init) {
    Message(MESSAGE_ERR, "Cannot allocate module layer init\n");
    Error(CORE_ERR_MEM);
  }

  l->setup = calloc(1, sizeof(setup));
  if (!l->setup) {
    Message(MESSAGE_ERR, "Cannot allocate module layer setup\n");
    Error(CORE_ERR_MEM);
  }

  l->setup->options = NULL;
  l->setup->head = NULL;
  l->setup->popt = NULL;

  l->handler = NULL;
  l->mode_handler = NULL;

  return l;
}

/**
 * @brief Wrapper to dlopen()
 *
 * @param name The name of the module to load
 *
 * @return The module pointer, NULL otherwise
 */
module* ModuleLoad(char *name) {
  module *m = NULL;
  char *fname = NULL;

  m = calloc(1, sizeof(module));
  if (!m) {
    Message(MESSAGE_ERR, "Cannot allocate memory for the module handler\n");
    Error(CORE_ERR_MEM);
  }

  m->layer = NULL;
  m->fallback = NULL;

  m->layer = LayerLoad();
  m->fallback = LayerLoad();

  fname = Name(MECHANIC_MODULE_PREFIX, name, "", LIBEXT);

  m->layer->handler = dlopen(fname, RTLD_NOW|RTLD_GLOBAL);
  if (!m->layer->handler) {
    Message(MESSAGE_ERR, "Cannot load module '%s': %s\n", name, dlerror());
    Error(CORE_ERR_MODULE);
  }

  free(fname);

  return m;
}

/**
 * @brief Allocates initial memory for the Layer
 *
 * - Load fallback layer function
 * - If the layer function exists, overwrite fallback
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
 */
int ModuleInit(module *m) {
  query *q = NULL;
  unsigned int opts = 0;
  int mstat = SUCCESS;

  /* Load fallback layer, at least core module must implement this */
  if (m->fallback) {
    if (m->fallback->handler) {
      m->layer->init = m->fallback->init;
    }
  }

  q = LoadSym(m, "Init", NO_FALLBACK);
  if (q) mstat = q(m->layer->init);
  CheckStatus(mstat);

  opts = m->layer->init->options;
  if (m->fallback) {
    if (m->fallback->handler) opts = opts + m->fallback->init->options;
  }

  m->layer->setup->options = calloc(opts, sizeof(options));
  if (!m->layer->setup->options) Error(CORE_ERR_MEM);

  m->layer->setup->popt = calloc(1, sizeof(popt));
  if (!m->layer->setup->popt) Error(CORE_ERR_MEM);

  m->layer->setup->popt->popt = NULL;
  m->layer->setup->popt->poptcontext = NULL;

  m->layer->setup->popt->popt = calloc(opts, sizeof(struct poptOption));
  if (!m->layer->setup->popt->popt) Error(CORE_ERR_MEM);

  m->layer->setup->popt->string_args = calloc(opts, sizeof(char*));
  if (!m->layer->setup->popt->string_args) Error(CORE_ERR_MEM);

  m->layer->setup->popt->val_args = calloc(opts, sizeof(int));
  if (!m->layer->setup->popt->val_args) Error(CORE_ERR_MEM);

  m->layer->setup->popt->int_args = calloc(opts, sizeof(int));
  if (!m->layer->setup->popt->int_args) Error(CORE_ERR_MEM);

  m->layer->setup->popt->long_args = calloc(opts, sizeof(long));
  if (!m->layer->setup->popt->long_args) Error(CORE_ERR_MEM);

  m->layer->setup->popt->float_args = calloc(opts, sizeof(float));
  if (!m->layer->setup->popt->float_args) Error(CORE_ERR_MEM);

  m->layer->setup->popt->double_args = calloc(opts, sizeof(double));
  if (!m->layer->setup->popt->double_args) Error(CORE_ERR_MEM);

  m->filename = NULL;
  m->communication_type = MPI_BLOCKING;

  /**
   * General Init fixes:
   * - we need at least one task pool
   * - we need at least one storage bank per pool
   * - we need at least one storage bank per task
   * - we need at least one attribute bank per dataset
   */
  if (m->layer->init->pools < 1) m->layer->init->pools = 1;
  if (m->layer->init->banks_per_pool < 1) m->layer->init->banks_per_pool = 1;
  if (m->layer->init->banks_per_task < 1) m->layer->init->banks_per_task = 1;
  if (m->layer->init->attr_per_dataset < 1) m->layer->init->attr_per_dataset = 1;
  if (m->layer->init->compound_fields < 1) m->layer->init->compound_fields = 1;

  return mstat;
}

/**
 * @brief Initializes the Setup
 *
 * This function calls the Setup() from the module to initialize the Config default option
 * structure. It merges with the fallback structure, so that all setup is available in one
 * layer (and only one config file can be used, with i.e. core setup included, but not
 * necessary).
 *
 * @param m The module pointer
 * @param argc The command line argc table
 * @param argv The command line argv table
 *
 * @return 0 on success, error code otherwise
 */
int ModuleSetup(module *m, int argc, char **argv) {
  query *q = NULL;
  int mstat = SUCCESS;

  q = LoadSym(m, "Setup", NO_FALLBACK);
  if (q) {
    mstat = q(m->layer->setup);
    CheckStatus(mstat);

    /**
     * Note:
     * Since we merge core layer to module layer, the user cannot change the core defaults
     */
    if (m->fallback) {
      if (m->fallback->handler) {
        mstat = ConfigMergeDefaults(m->layer->setup->options, m->fallback->setup->options);
        CheckStatus(mstat);
      }
    }
  }

  m->layer->setup->head = ConfigAssignDefaults(m->layer->setup->options);

  /* Popt options */
  mstat = PoptOptions(m, m->layer->setup);
  CheckStatus(mstat);
  m->layer->setup->popt->poptcontext = poptGetContext(NULL, argc, (const char **) argv, m->layer->setup->popt->popt, 0);
  poptGetNextOpt(m->layer->setup->popt->poptcontext);

  ConfigUpdate(m->layer->setup);

  return mstat;
}

/**
 * @brief Finalize the layer
 *
 * @param l The layer pointer to finalize
 */
void FinalizeLayer(layer *l) {
  if (l) {
    if (l->handler) dlclose(l->handler);
    if (l->mode_handler) dlclose(l->mode_handler);
    if (l->setup) {
      if (l->setup->options) free(l->setup->options);
      if (l->setup->popt) {
        if (l->setup->popt->popt) free(l->setup->popt->popt);
        if (l->setup->popt->string_args) free(l->setup->popt->string_args);
        if (l->setup->popt->val_args) free(l->setup->popt->val_args);
        if (l->setup->popt->int_args) free(l->setup->popt->int_args);
        if (l->setup->popt->long_args) free(l->setup->popt->long_args);
        if (l->setup->popt->float_args) free(l->setup->popt->float_args);
        if (l->setup->popt->double_args) free(l->setup->popt->double_args);
        if (l->setup->popt->poptcontext) poptFreeContext(l->setup->popt->poptcontext);
        free(l->setup->popt);
      }
      if (l->setup->head) ConfigCleanup(l->setup->head);

      free(l->setup);
    }

    if (l->init) free(l->init);

    free(l);
  }
}

/**
 * @brief Finalize the module
 *
 * @param m The module pointer to finalize
 */
void ModuleFinalize(module *m) {
  if (m) {
    if (m->fallback) FinalizeLayer(m->fallback);
    if (m->layer) FinalizeLayer(m->layer);
    if (m->filename) free(m->filename);
    free(m);
  }
}

