/**
 * C and Fortran interface
 * =======================
 *
 * In this example, C-function `cfunc` is passed to the Fortran subroutine `integrator`.
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -c mechanic_module_ffc.c -fPIC -Dpic
 *    mpif90 -std=f2003 -c mechanic_module_ffc_fortran.F90 -fPIC -Dpic
 *    mpicc -shared -o libmechanic_module_ffc.so mechanic_module_ffc.o mechanic_module_ffc_fortran.o -lgfortran -lmechanic2
 */
#include "Mechanic2.h"

typedef int (*cfunc_ptr)(int n, double *x);

/* Function to use in Fortran */
int cfunc(int n, double *x) {
  Message(MESSAGE_OUTPUT, "cfunc x[7] = %f\n", x[7]);
  return 9;
}

/* Fortran interface */
void integrator(cfunc_ptr func, int n, double *ctrl, double *x, double *f, int *pstatus);

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int n, pstatus;
  double ctrl[10], x[24], f;

  n = t->tid;
  ctrl[7] = t->location[0];
  x[21] = t->location[1];
  f = p->pid;

  /* Call Fortran function, we pass cfunc as an argument */
  integrator(cfunc, n, ctrl, x, &f, &pstatus);

  Message(MESSAGE_OUTPUT, "Node %d, status = %d\n", t->node, pstatus);
  return SUCCESS;
}

