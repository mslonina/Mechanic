/**
 * @file
 * The module interface
 */
#include "MModules.h"

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
query* LoadSym(module *m, char* function, int flag) {
  char *err;
  void *handler;
  void *fallback;
  query* q = NULL;

  /* Reset dlerror() */
  dlerror();

  handler = m->layer.handler;
  fallback = m->fallback.handler;

  if (flag == FALLBACK_ONLY) {
    handler = m->fallback.handler;
    fallback = NULL;
  }

  Message(MESSAGE_DEBUG, "Querying function '%s'\n", function);
  q = (query*) dlsym(handler, function);
  err = dlerror();

  if (err == NULL) {
    Message(MESSAGE_DEBUG, "Loading function '%s'\n", function);
    return q;
  } else {
    if (fallback) {
      Message(MESSAGE_DEBUG, "Querying fallback function '%s'\n", function);
      q = (query*) dlsym(fallback, function);
      err = dlerror();
      if (err == NULL) {
        Message(MESSAGE_DEBUG, "Loading fallback function '%s'\n", function);
      return q;
      }
    }
  }

  return q;
}

