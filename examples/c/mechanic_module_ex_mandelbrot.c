/**
 * The Mandelbrot set
 * ==================
 *
 * This example shows how to compute the Mandelbrot set
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_mandelbrot.c -o libmechanic_module_ex_mandelbrot.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_mandelbrot -x 2048 -y 2048
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 */
#include "mechanic.h"

int fractal(double a, double b, double c);

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .name = "result",
    .rank = TASK_BOARD_RANK,
    .dim[0] = 1,
    .dim[1] = 1,
    .dim[2] = 1,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
    .datatype = H5T_NATIVE_DOUBLE
  };

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double real_min, real_max, imag_min, imag_max;
  double scale_real, scale_imag;
  double c, xres, yres, zres;
  double x,y;
  double buffer[1][1][1];

  real_min = -2.0;
  real_max = 2.0;
  imag_min = -2.0;
  imag_max = 2.0;
  c = 4.0;

  xres = p->board->layout.dim[1];
  yres = p->board->layout.dim[0];
  zres = p->board->layout.dim[2];

  /* Coordinate system */
  scale_real = (real_max - real_min) / ((double) xres - 1.0);
  scale_imag = (imag_max - imag_min) / ((double) yres - 1.0);

  // The vertical (Im) position of the pixel
  x = imag_min + t->location[0] * scale_imag;

  // The horizontal (Re) position of the pixel
  y = real_min + t->location[1] * scale_real;

  // The state of the system
  buffer[0][0][0] = fractal(x, y, c);

  MWriteData(t, "result", buffer);

  return SUCCESS;
}

/**
 * Computes the value for the given pixel position
 */
int fractal(double a, double b, double c){

  double temp, lengthsq;
  int max_iter = 256;
  int count = 0;
  double zr = 0.0, zi = 0.0;

  do {

    temp = zr*zr - zi*zi + a;
    zi = 2*zr*zi + b;
    zr = temp;
    lengthsq = zr*zr + zi*zi;
    count++;

  } while ((lengthsq < c) && (count < max_iter));

  return count;
}

