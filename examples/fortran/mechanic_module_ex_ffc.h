/**
 * C and Fortran interface
 * =======================
 *
 */
#ifndef MECHANIC_MODULE_FFC_H

typedef int (*cfunc_ptr)(int n, double *x);

/* Fortran interface */
void integrator(cfunc_ptr func, int n, double *ctrl, double *x, double *f, int *pstatus);

#endif

