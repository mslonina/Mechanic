#ifndef MECHANIC_COMMON_H
#define MECHANIC_COMMON_H

#include "MMechanic2.h"

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#define MASTER 0
#define NORMAL_MODE 0
#define RESTART_MODE 1

enum {
  MESSAGE_INFO,
  MESSAGE_ERR,
  MESSAGE_IERR,
  MESSAGE_CONT,
  MESSAGE_CONT2,
  MESSAGE_WARN,
	MESSAGE_DEBUG
} MessageType;

char* Filename(char *prefix, char *name, char *suffix, char *extension);
void Message(int type, char* message, ...);
void Error(int status);
void CheckStatus(int status);

double* AllocateDoubleVec(int *dims);
double** AllocateDoubleArray(int *dims);
void FreeDoubleVec(double *vec, int *dims);
void FreeDoubleArray(double **array, int *dims);

#endif
