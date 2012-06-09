/**
 * @file
 * The Mechanic2 public interface
 */
#include "MMechanic2.h"

/**
 * @brief Common messaging interface
 *
 * @param type The type of the message
 * @param message The message to display
 */
void Message(int type, char *message, ...) {
  static char message2[2048];
  va_list args;

  va_start(args, message);
    vsprintf(message2, message, args);
    if (type == MESSAGE_INFO)    printf("-- %s", message2);
    if (type == MESSAGE_COMMENT) printf("#  %s", message2);
    if (type == MESSAGE_OUTPUT) printf("   %s", message2);
    if (type == MESSAGE_ERR) printf("!! %s", message2);
		if (type == MESSAGE_WARN) printf(".. %s", message2);
  va_end(args);
}

