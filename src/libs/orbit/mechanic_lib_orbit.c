/*
 * MECHANIC
 * Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 *
 * This file is part of MECHANIC code.
 *
 * MECHANIC was created to help solving many numerical problems by providing
 * tools for improving scalability and functionality of the code. MECHANIC was
 * released in belief it will be useful. If you are going to use this code, or
 * its parts, please consider referring to the authors either by the website
 * or the user guide reference.
 *
 * http://mechanics.astri.umk.pl/projects/mechanic
 *
 * User guide should be provided with the package or
 * http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Nicolaus Copernicus University nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_lib_orbit.h"

/**
 * ORBITAL ELEMENTS CONVERSION
 *
 * The input arrays should have following shape:
 *
 * elements[0] - a
 * elements[1] - e
 * elements[2] - i
 * elements[3] - capomega
 * elements[4] - omega
 * elements[5] - mean anomally
 *
 * rv[0] - x
 * rv[1] - y
 * rv[2] - z
 * rv[3] - vx
 * rv[4] - vy
 * rv[5] - vz
 *
 * @return
 * 0 on success, errcode otherwise
 *
 */

/**
 * @fn double orbit_kepler(double e, double m)
 * @brief Solves Kepler's equation with Danby's approach
 *
 * See J.M.A. Danby, "The Solution of Kepler's Equation, III"
 * Cel. Mech. 40 (1987) pp. 303-312
 *
 * To obtain Danby's precision, set precison = MECHANIC_ORBIT_DANBY.
 * To obtain much more accurate solution, set precision =
 * MECHANIC_ORBIT_ACCURATE.
 *
 */
double orbit_kepler(int precision, double e, double m){

  double E;
  int i;

  /* Danby's inital guess */
  if (m >= 0.1){
    E = m + 0.85 * sin(m) * e;
  } else {
    E = m + (pow(6.0 * m, (1.0/3.0)) - m * pow(e, 2));
  }

  E = orbit_kepler_iteration(precision, e, m, E);

  return E;
}

double orbit_kepler_iteration(int precision, double e, double m, double E){

  double dE, f0, f1, f2, f3, absf;
  int flag, i;

  i = 0;
  flag = 0;

  do {

    f0 = E - e * sin(E) - m;
    f1 = 1.0 - e * cos(E);
    f2 = e * sin(E);
    f3 = e * cos(E);

    dE = - f0 / f1;
    dE = - f0 / (f1 + 0.5 * dE * f2);
    dE = - f0 / (f1 + 0.5 * dE * f2 + pow(dE, 2) * f3 / 6.0);

    E = E + dE;

    /* Break the loop according to given precision */
    if (precision == MECHANIC_ORBIT_DANBY) {
      i++;
      if (i >= 2) flag = 1;
    }

    if (precision == MECHANIC_ORBIT_ACCURATE) {
      absf = fabs(f0);
      if (absf >= MECHANIC_ORBIT_ACCURACY_LEVEL) flag = 1;
    }

  } while (flag == 0);

  return E;
}

/* Converts orbital elements to rv frame */
int orbit_el2rv(int orbit_type, double gm, double el[], double rv[]){

  double matrix[6], r, unit, s2e, s2g;
  double sin_inc, cos_inc, sin_omega, cos_omega, sin_capom, cos_capom;
  double E, r1, r2, v1, v2, divider;

  unit = 1.0;

  /* Solve Kepler's equation */
  E = orbit_kepler(MECHANIC_ORBIT_DANBY, el[1], el[5]);

  /* P, Q matrix */
  sin_inc = sin(el[2]);
  cos_inc = cos(el[2]);

  sin_capom = sin(el[3]);
  cos_capom = cos(el[3]);

  sin_omega = sin(el[4]);
  cos_omega = cos(el[4]);

  matrix[0] = cos_omega * cos_capom - sin_omega * cos_inc * sin_capom;
  matrix[1] = cos_omega * sin_capom + sin_omega * cos_inc * cos_capom;
  matrix[2] = sin_omega * sin_inc;
  matrix[3] = - sin_omega * cos_capom - cos_omega * cos_inc * sin_capom;
  matrix[4] = - sin_omega * sin_capom + cos_omega * cos_inc * cos_capom;
  matrix[5] = cos_omega * sin_inc;

  /* Helpers */
  s2e = sqrt(unit - pow(el[1], 2));
  s2g = sqrt(gm * el[0]);

  r1 = el[0] * (cos(E) - el[1]);
  r2 = el[0] * s2e * sin(E);

  divider = el[0] * (unit - el[1] * cos(E));
  v1 = - sin(E) * s2g / divider;
  v2 = s2g * s2e * cos(E) / divider;

  /* Final conversion */
  rv[0] = matrix[0] * r1 + matrix[3] * r2;
  rv[1] = matrix[1] * r1 + matrix[4] * r2;
  rv[2] = matrix[2] * r1 + matrix[5] * r2;
  rv[3] = matrix[0] * v1 + matrix[3] * v2;
  rv[4] = matrix[1] * v1 + matrix[4] * v2;
  rv[5] = matrix[2] * v1 + matrix[5] * v2;

  return 0;
}

/* COORDINATES CONVERSION */

int orbit_kepler2cart(int direction){

  return 0;
}

int orbit_baryxv2baryrp(int direction){

  return 0;
}

/**
 * Author: K. Gozdziewski (2001)
 */
int orbit_helio2bary(int direction){

  return 0;
}

/**
 * Author: K. Gozdziewski (2001)
 */
int orbit_helio2poincare(int direction){

  return 0;
}

