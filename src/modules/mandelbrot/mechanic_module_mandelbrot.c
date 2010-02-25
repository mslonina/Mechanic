#include "mechanic.h"
#include "mechanic_module_mandelbrot.h"

int mandelbrot_generateFractal(double a, double b, double c);

int mandelbrot_init(moduleInfo *md){

  md->name = "mandelbrot";
  md->author = "MSlonina";
  md->date = "2010";
  md->version = "1.0";

  return 0;
}
int mandelbrot_query(){
  return 0;
}
int mandelbrot_cleanup(){
  return 0;
}

int mandelbrot_pixelCompute(int slave, configData* d, masterData* r){

  int i = 0;
  double real_min, real_max, imag_min, imag_max;
  double scale_real, scale_imag;
  double c, zoom, offx, offy;

  real_min = -2.0;
  real_max = 2.0;
  imag_min = -2.0;
  imag_max = 2.0;
  c = 4.0;
  zoom = 1.0;
  offx = 0.0;
  offy = 0.0;

  //coordinate system
  scale_real = (real_max - real_min)/(zoom*((double)d->xres - 1.0));
  scale_imag = (imag_max - imag_min)/(zoom*((double)d->yres - 1.0));
  
  r->res[0] = real_min + r->coords[0]*scale_real;
  r->res[1] = imag_max - r->coords[1]*scale_imag;

  //Mandelbrot set
  r->res[2] = mandelbrot_generateFractal(r->res[0], r->res[1], c);

   return 0;
}

int mandelbrot_generateFractal(double a, double b, double c){
 
  double temp, lengthsq;
  int max_iter = 256;
  int count = 0;
  double zr = 0.0, zi = 0.0;
  
  do{

    temp = zr*zr - zi*zi + a;
    zi = 2*zr*zi + b;
    zr = temp;
    lengthsq = zr*zr + zi*zi;
    count++;

  } while ((lengthsq < c) && (count < max_iter));
 
  return count;
}

