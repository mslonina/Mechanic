/**
 * @file
 * The Arnold Web module for Mechanic: The Arnold Web part
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mechanic_module_aweb.h"

/**
 * @function
 * The right hand sides + variational equations of the Hamiltonian model of the Arnold web, 
 * see Froeschle+ Science 289 (2000)
 */
void vinteraction(double *y,  double *a, double *dy, double *v, double eps) {
  double I1, I2, I3, f1, f2, f3, sf1, sf2;
  double sf3, cf1, cf2, cf3, dif, dif2, dif3, sum;

  f1   = y[0];
  f2   = y[1];
  f3   = y[2];
  I1   = y[3];
  I2   = y[4];
  I3   = y[5];

  sf1  = sin(f1);
  sf2  = sin(f2);
  sf3  = sin(f3);
  cf1  = cos(f1);
  cf2  = cos(f2);
  cf3  = cos(f3);

  dif  = cf1 + cf2 + cf3 + 4;
  dif2 = eps/(dif*dif);
  dif3 = dif2/dif;

  // right hand sides
  a[0] =  I1;
  a[1] =  I2;
  a[2] =  1.0;
  a[3] = -sf1*dif2;
  a[4] = -sf2*dif2;
  a[5] = -sf3*dif2;

  // variational equations
  v[0] =  1;
  v[1] =  1;
  v[2] =  0;

  sum  = 2*(sf1*dy[0] + sf2*dy[1] + sf3*dy[2])*dif3;

  v[3] = -cf1*dif2*dy[0] - sum*sf1;
  v[4] = -cf2*dif2*dy[1] - sum*sf2;
  v[5] = -cf3*dif2*dy[2] - sum*sf3;

}

/**
 * @function
 * The energy integral
 */
double energy(double *y, double eps) {
  double I1, I2, I3, f1, f2, f3, cf1, cf2, cf3, dif, en;

  f1   = y[0];
  f2   = y[1];
  f3   = y[2];
  I1   = y[3];
  I2   = y[4];
  I3   = y[5];

  cf1  = cos(f1);
  cf2  = cos(f2);
  cf3  = cos(f3);

  dif  = eps/(cf1 + cf2 + cf3 + 4);
  en   = I1*I1/2.0 + I2*I2/2.0 + I3 + dif;

  return en;
}

/**
 * @function
 * The variational integral
 */
double variat(double *xv, double *dy, double eps) {
  double acc[6], var[6], vint;

  vinteraction(xv, acc, dy, var, eps);

  vint = -acc[3]*dy[0] -acc[4]*dy[1] -acc[5]*dy[2] +
         +acc[0]*dy[3] +acc[1]*dy[4] +acc[2]*dy[5];

  return vint;
}

/**
 * @function
 * Normalizes the variational vector (flag = 1)
 */
double norm(int dim, double *a, int flag) {
  double tmp = 0.0;
  int i;

  for (i=1; i<=dim; i++) tmp += a[i]*a[i];
  tmp = sqrt(tmp);
  if (flag) for (i=1; i<=dim; i++) a[i]= a[i]/tmp;

  return tmp;
}

/**
 * @function
 * Symplectic MEGNO (Gozdziewski, Breiter & Borczyk, MNRAS, 2008)
 * with the modified Leapfrog integrator SABA2 (Laskar & Robutel, CMDA, 2001)
 */
double smegno2(double *xv0, double step, double tend, double eps, double *err) {
  double c1, c2, d1;
  double Y0, mY0, Y1, mY1, maxe;
  double acc[6], dy[6], var[6], xv[6], t, en, en0, h, delta, delta0;
  long int ks;
  int i, checkout;
  //double v0;

  c1    = 0.5-sqrt(3.0)/10.0;
  c2    = sqrt(3.0)/3.0;
  d1    = 0.5;

  t     = 0.0;
  maxe  = 0.0;
  checkout = 1000;
  Y0    = mY0   = Y1    = mY1   = 0.0;

  /* Initialize state vector */
  for (i=0; i<=6; i++) xv[i] = xv0[i];

  /* Set the tangent vector */
  for (i=0; i<=6; i++) dy[i] = rand()/(RAND_MAX+1.0);

  /* Normalize the tangent vector */
  delta0= norm(6, dy, 1);
  en0   = energy(xv, eps);
  //v0    = variat(xv, dy, eps);

  /* SABA2 integrator + variations */

  ks    = 0;

  while (t <= tend) {

    /* 1st drift */
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    /* 1st kick */
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    /* 2nd drift */
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    ks++;
    t = ks*step;

    /* MEGNO */
    delta   = norm(6, dy, 0);
    Y1      =  Y0*((double)ks-1.0)/((double)ks) + 2.0*log(delta/delta0);
    mY1     = mY0*((double)ks-1.0)/((double)ks) + Y1/((double)ks);
    Y0      = Y1;
    mY0     = mY1;
    delta0  = delta;

    /* relative errors of the energy and the variational integrator */
    if (ks%checkout == 0) {
      en = fabs((energy(xv,eps)-en0)/en0);
      if (en>maxe) maxe = en;
    }
  }

  *err = maxe;
  return mY1;
}

/**
 * @function
 * Symplectic MEGNO (Gozdziewski, Breiter & Borczyk, MNRAS, 2008)
 * with the modified Leapfrog integrator SABA2 (Laskar & Robutel, CMDA, 2001)
 */
double smegno3(double *xv0, double step, double tend, double eps,  double *err) {
  double c1, c2, d1, d2;
  double Y0, mY0, Y1, mY1, maxe;
  double acc[6], dy[6], var[6], xv[6], t, en, en0, h, delta, delta0;
  long int ks;
  int i, checkout;
  //double v0;

  // SABA3 coefficients
  c1    = 0.5 - sqrt(15.0)/10.0;
  c2    = sqrt(15.0)/10.0;
  d1    = 5.0/18.0;
  d2    = 4.0/9.0;

  t     = 0.0;
  maxe  = 0.0;
  checkout = 1000;
  Y0    = mY0   = Y1    = mY1   = 0.0;

  /* Initialize state vector */
  for (i=0; i<=6; i++) xv[i] = xv0[i];

  /* Set the tangent vector */
  for (i=0; i<=6; i++) dy[i] = rand()/(RAND_MAX+1.0);

  /* Randomize the tangent vector */
  delta0= norm(6, dy, 1);

  en0   = energy(xv, eps);
  //v0    = variat(xv, dy, eps);

  /* SABA3 integrator + variations */
  ks    = 0;

  while (t <= tend) {

    // 1st drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 1st kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 2nd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 2nd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d2*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 3rd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 3rd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // final drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    ks++;
    t = ks*step;

    /* MEGNO */
    delta   = norm(6, dy, 0);
    Y1      =  Y0*((double)ks-1.0)/((double)ks) + 2.0*log(delta/delta0);
    mY1     = mY0*((double)ks-1.0)/((double)ks) + Y1/((double)ks);
    Y0      = Y1;
    mY0     = mY1;
    delta0  = delta;

    // relative errors of the energy and the variational integrator
    if (ks%checkout == 0) {
      en = fabs((energy(xv,eps)-en0)/en0);
      if (en>maxe) maxe = en;
    }
  }

  *err = maxe;
  return mY1;
}

