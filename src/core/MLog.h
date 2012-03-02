/**
 * Log-interface
 */
#ifndef MECHANIC_LOG_H
#define MECHANIC_LOG_H

#include "MMechanic2.h"

enum {
  MESSAGE_INFO,
  MESSAGE_ERR,
  MESSAGE_IERR,
  MESSAGE_CONT,
  MESSAGE_CONT2,
  MESSAGE_WARN,
	MESSAGE_DEBUG
} MessageType;

void Message(int type, char* message, ...);
void Error(int status);
void CheckStatus(int status);

#endif
