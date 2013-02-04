#ifndef MECHANIC_M2W_PUBLIC_H
#define MECHANIC_M2W_PUBLIC_H

#include "M2Apublic.h"
#include "M2Epublic.h"
#include "M2Hpublic.h"
#include "M2Mpublic.h"
#include "M2Spublic.h"
#include "M2Tpublic.h"
#include "M2Ppublic.h"

int M2Prepare(module *m);
int M2Process(module *m, pool **all);
int M2NodePrepare(module *m, pool **all, pool *current);
int M2NodeProcess(module *m, pool **all, pool *current);
int M2LoopPrepare(module *m, pool **all, pool *current);
int M2LoopProcess(module *m, pool **all, pool *current);
int M2Send(int node, int dest, int tag, module *m, pool *p);
int M2Receive(int node, int sender, int tag, module *m, pool *p, void *buffer);

int Pack(module *m, void *buffer, pool *p, task *t, int tag);
int Unpack(module *m, void *buffer, pool *p, task *t, int *tag);

#endif
