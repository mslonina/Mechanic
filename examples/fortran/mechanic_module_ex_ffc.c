/**
 * C and Fortran interface
 * =======================
 *
 * In this example, C-function `cfunc` is passed to the Fortran subroutine `integrator`.
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -lmechanic -c mechanic_module_ex_ffc.c 
 *    mpif90 -std=f2003 -fPIC -Dpic -c mechanic_module_ex_ffc_fortran.F90 
 *    mpicc -shared -lgfortran -lmechanic -o libmechanic_module_ex_ffc.so \
 *        mechanic_module_ex_ffc.o mechanic_module_ex_ffc_fortran.o
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_ffc -x 10 -y 20
 *
 */
#include "mechanic.h"

typedef int (*cfunc_ptr)(int n, double *x);

/* Function to use in Fortran */
int cfunc(int n, double *x) {
  Message(MESSAGE_OUTPUT, "cfunc x[7] = %f\n", x[7]);
  return 9;
}

/* Fortran interface */
void integrator(cfunc_ptr func, int n, double *ctrl, double *x, double *f, int *pstatus);

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .name = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 10,
    .datatype = H5T_NATIVE_DOUBLE,
    .storage_type = STORAGE_PM3D,
    .use_hdf = 1,
  };
  return SUCCESS;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int n, pstatus;
  double ctrl[10], x[24], f;
  double buffer[1][10];

  n = t->tid;
  ctrl[7] = t->location[0];
  x[21] = t->location[1];
  f = p->pid;

  /* Call Fortran function, we pass cfunc as an argument */
  integrator(cfunc, n, ctrl, x, &f, &pstatus);

  buffer[0][0] = ctrl[7];
  buffer[0][1] = x[21];
  buffer[0][2] = f;
  buffer[0][3] = n;

  MWriteData(t, "result", buffer);

  Message(MESSAGE_OUTPUT, "Node %d, status = %d\n", t->node, pstatus);
  return SUCCESS;
}

