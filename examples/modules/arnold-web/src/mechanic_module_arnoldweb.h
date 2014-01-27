#ifndef MECHANIC_MODULE_ARNOLDWEB_H
#define MECHANIC_MODULE_ARNOLDWEB_H

#include <math.h>

#define MAX_SETS 10
#define MAX_SNAPSHOTS 10
#define CRITICAL_MEGNO 5.0

double norm(int dim, double *a, int flag);
double variat(double *xv, double *dy, double eps);
double energy(double *y, double eps);
void vinteraction(double *y,  double *a, double *dy, double *v, double eps);
double smegno2(double *state, double step, unsigned long int *loop_step, 
  double tstart, double tend, double eps, double *err);
double smegno3(double *state, double step, unsigned long int *loop_step,
  double tstart, double tend, double eps, double *err);
double smegno4(double *state, double step, unsigned long int *loop_step,
  double tstart, double tend, double eps, double *err);

#endif
