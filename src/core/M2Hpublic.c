/**
 * @file
 * Module handlers (public API)
 */
#include "M2Hpublic.h"

/**
 * @brief Wrapper to dlsym()
 *
 * This function loads the specified function from the module. By default the core module
 * is queried at first, and then the user specified module (LOAD_DEFAULT flag). If the
 * core module has to be loaded only, use FALLBACK_ONLY flag.
 *
 * @param m The module pointer
 * @param function The function to load
 * @param flag One of the flags: LOAD_DEFAULT or FALLBACK_ONLY
 *
 * @return The function handler
 */
query* LoadSym(module *m, char *function, int flag) {
  char *err;
  void *handler = NULL;
  void *fallback = NULL;
  query *q = NULL;

  /* Reset dlerror() */
  dlerror();

  if (!m->layer) {
    Message(MESSAGE_ERR, "LoadSym: no module layer\n");
    Error(CORE_ERR_MEM);
  } else {
    if (!m->layer->handler) {
      Message(MESSAGE_ERR, "LoadSym: no module layer handler\n");
      Error(CORE_ERR_MEM);
    } else {
      handler = m->layer->handler;
    }
  }

  if (m->fallback) {
    if (m->fallback->handler) {
      fallback = m->fallback->handler;
    }
  }

  if (flag == FALLBACK_ONLY) {
    if (!m->fallback) {
      Message(MESSAGE_ERR, "LoadSym: FALLBACK USED but no fallback layer found\n");
      Error(CORE_ERR_MEM);
    } else {
      if (!m->fallback->handler) {
        Message(MESSAGE_ERR, "LoadSym: FALLBACK USED but no fallback layer handler found\n");
        Error(CORE_ERR_MEM);
      } else {
        handler = m->fallback->handler;
        fallback = NULL;
      }
    }
  }

  q = (query*) dlsym(handler, function);
  err = dlerror();

  if (err == NULL) {
    return q;
  } else {
    if (fallback) {
      q = (query*) dlsym(fallback, function);
      err = dlerror();
      if (err == NULL) {
        return q;
      } else {
        Message(MESSAGE_ERR, "This should never happen on fallback function '%s'\n", function);
        Error(CORE_ERR_MEM);
      }
    }
  }

  return q;
}

