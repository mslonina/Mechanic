/**
 * @file
 */

#include "MModules.h"

/**
 * @function
 * Wrapper to dlsym()
 *
 * @return
 * Function handler
 */
query* LoadSym(module *m, char* function, int flag) {
  query* q = NULL;
  char *err;
  void *handler;
  void *fallback;

  /* Reset dlerror() */
  dlerror();

  handler = m->layer.handler;
  fallback = m->fallback.handler;

  if (flag == FALLBACK_ONLY) {
    handler = m->fallback.handler;
    fallback = NULL;
  }

  Message(MESSAGE_INFO, "Querying function '%s'\n", function);
  q = (query*) dlsym(handler, function);
  err = dlerror();

  if (err == NULL) {
    Message(MESSAGE_INFO, "Loading function '%s'\n", function);
    return q;
  } else {
    if (fallback) {
      Message(MESSAGE_INFO, "Querying fallback function '%s'\n", function);
      q = (query*) dlsym(fallback, function);
      err = dlerror();
      if (err == NULL) {
        Message(MESSAGE_INFO, "Loading fallback function '%s'\n", function);
      return q;
      }
    }
  }

  return q;
}


