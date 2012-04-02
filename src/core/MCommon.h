#ifndef MECHANIC_COMMON_H
#define MECHANIC_COMMON_H

#include "MMechanic2.h"

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#define MASTER 0
#define NORMAL_MODE 0
#define RESTART_MODE 1

#define DEFAULT_CONFIG_FILE "mechanic-config.cfg"

enum {
  MESSAGE_INFO,
  MESSAGE_ERR,
  MESSAGE_IERR,
  MESSAGE_CONT,
  MESSAGE_CONT2,
  MESSAGE_WARN,
	MESSAGE_DEBUG
} MessageType;

char* Name(char *prefix, char *name, char *suffix, char *extension);
void Message(int type, char* message, ...);
void Error(int status);
void Abort(int status);
void CheckStatus(int status);

double** AllocateBuffer(int rank, int *dims);
void FreeBuffer(double **array, int *dims);
int GetSize(int rank, int *dims);

void Array2Vec(double *vec, double **array, int rank, int *dims);
void Vec2Array(double *vec, double **array, int rank, int *dims);

#endif
