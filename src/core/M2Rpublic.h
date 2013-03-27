/**
 * @file
 * The restart mode (public API)
 */
#ifndef MECHANIC_M2R_PUBLIC_H
#define MECHANIC_M2R_PUBLIC_H

#include "M2Apublic.h"
#include "M2Epublic.h"
#include "M2Hpublic.h"
#include "M2Mpublic.h"
#include "M2Spublic.h"
#include "M2Tpublic.h"
#include "M2Ppublic.h"

/**
 * @struct checkpoint
 * The checkpoint
 */
typedef struct {
  int cid; /**< The checkpoint id */
  unsigned int counter; /**< The checkpoint internal counter */
  unsigned int size; /**< The actual checkpoint size */
  storage *storage; /**< The checkpoint data */
} checkpoint;

checkpoint* CheckpointLoad(module *m, pool *p, int cid);
int M2CheckpointPrepare(module *m, pool *p, checkpoint *c);
int CheckpointProcess(module *m, pool *p, checkpoint *c);
void CheckpointReset(module *m, pool *p, checkpoint *c, int cid);
void CheckpointFinalize(module *m, pool *p, checkpoint *c);
int Backup(module *m, pool *p);

#endif
