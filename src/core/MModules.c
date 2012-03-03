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
query* LoadSym(void *handler, char* function) {
  query* q = NULL;
  char *err;

  /* Reset dlerror() */
  dlerror();

  Message(MESSAGE_INFO, "Querying function '%s'\n", function);
  q = (query*) dlsym(handler, function);
  err = dlerror();

  if (err == NULL) {
    Message(MESSAGE_INFO, "Loading function '%s'\n", function);
    return q;
  }

  return q;
}


