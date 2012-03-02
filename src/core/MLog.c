/**
 * @file
 */
#include "MLog.h"

/**
 * @function
 * Common messaging interface
 */
void Message(int type, char *message, ...) {

  int silent = 0;
  static char message2[2048];
  va_list args;

  va_start(args, message);
    vsprintf(message2, message, args);
    if (silent == 0) {
      if (type == MESSAGE_INFO) printf("-> %s", message2);
      if (type == MESSAGE_CONT) printf("   %s", message2);
      if (type == MESSAGE_CONT2) printf("   \t\t %s", message2);
		  if (type == MESSAGE_DEBUG) printf("   %s", message2);
		  //if (type == MESSAGE_DEBUG && debug == 1) printf("   %s", message2);
    }
      if (type == MESSAGE_ERR) perror(message2);
      if (type == MESSAGE_IERR) printf("!! %s", message2);
		  if (type == MESSAGE_WARN) printf(".. %s", message2);
  va_end(args);

}

void Error(int errcode) {
//  if (errcode == CORE_ERR_CORE) Abort(CORE_ERR_CORE);
}

void CheckStatus(int status) {

}
