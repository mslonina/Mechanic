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

/* [ORBIT] */

/**
 * @section orbit The Orbit Library
 *
 * The Orbit Library was created to handle common tasks meet in Celestial
 * Mechanics, i.e. orbital elements conversion. The library provides following
 * functions:
 *
 * @code
 * @icode libs/orbit/mechanic_lib_orbit.h ORBIT_API
 * @endcode
 *
 * The parameters are:
 * - @c el[] -- input/output orbital elements. The array should have following
 *   shape:
 *   @code
 *   el[0] - a
 *   el[1] - e
 *   el[2] - i [radians]
 *   el[3] - capomega [radians]
 *   el[4] - omega [radians]
 *   el[5] - mean anomally [radians]
 *   @endcode
 *
 * - @c rv[] -- input/output rv frame. The array should have following shape:
 *   @code
 *   rv[0] - x
 *   rv[1] - y
 *   rv[2] - z
 *   rv[3] - vx
 *   rv[4] - vy
 *   rv[5] - vz
 *   @endcode
 * - @c gm -- mass parameter
 * - @c e -- eccentricity
 * - @c m -- mean anomaly
 * - @c E -- initial solution for Kepler's equation
 * - @c precision -- the precision used to solve Kepler's equation
 * - @c direction -- direction of elements conversion, 1: el[] -> rv[], -1 rv[]
 *   -> el[]
 * - @c angle -- angle to convert
 *
 * The Kepler's equation is solved using Danby's approach, see J.M.A. Danby,
 * "The Solution of Kepler's Equation, III", Cel. Mech. 40 (1987) pp. 303-312.
 *
 * To use @c Orbit, you need to include @c mechanic/mechanic_lib_orbit.h in
 * your code and link it to @c libmechanic_orbit.so.
 */

/* [/ORBIT] */

/**
 * @fn double orbit_kepler(int precision, double e, double m)
 * @brief Solves Kepler's equation with Danby's approach
 *
 * To obtain Danby's precision, set precison = MECHANIC_ORBIT_DANBY.
 * To obtain much more accurate solution, set
 * precision = MECHANIC_ORBIT_ACCURATE.
 *
 */
double orbit_kepler(int precision, double e, double m){

  double E;

  /* Danby's inital guess */
  if (m >= 0.1) {
    E = m + 0.85 * sin(m) * e;
  } else {
    E = m + (pow(6.0 * m, (1.0/3.0)) - m * pow(e, 2.0));
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
    dE = - f0 / (f1 + 0.5 * dE * f2 + pow(dE, 2.0) * f3 / 6.0);

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

/**
 * @fn int orbit_el2rv(int precision, double gm, double el[], double rv[])
 *
 * @brief
 * Converts orbital elements to cartesian rv frame
 *
 * @return
 * Should return 0 on success, errcode otherwise
 */
int orbit_el2rv(int precision, double gm, double el[], double rv[]){

  double matrix[6], unit, s2e, s2g;
  double sin_inc, cos_inc, sin_omega, cos_omega, sin_capom, cos_capom;
  double E, r1, r2, v1, v2, divider;
  int orbit_type;

  unit = 1.0;

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

  if (el[1] < 0.0) {
    mechanic_message(MECHANIC_MESSAGE_WARN, "Eccentricity less than 0\n");
    mechanic_message(MECHANIC_MESSAGE_CONT, "Will set to 0.0\n");
    el[1] = 0.0;
  }

  if (el[1] < 1.0) orbit_type = MECHANIC_ORBIT_ELLIPSE;
  if (el[1] == 1.0) orbit_type = MECHANIC_ORBIT_PARABOLA;
  if (el[1] > 1.0) orbit_type = MECHANIC_ORBIT_HYPERBOLA;

  if (orbit_type == MECHANIC_ORBIT_ELLIPSE) {

    /* Solve Kepler's equation */
    E = orbit_kepler(precision, el[1], el[5]);

    s2e = sqrt(unit - pow(el[1], 2.0));
    s2g = sqrt(gm * el[0]);

    r1 = el[0] * (cos(E) - el[1]);
    r2 = el[0] * s2e * sin(E);

    divider = el[0] * (unit - el[1] * cos(E));
    v1 = - sin(E) * s2g / divider;
    v2 = s2g * s2e * cos(E) / divider;

  }

  if (orbit_type == MECHANIC_ORBIT_PARABOLA) {
  }

  if (orbit_type == MECHANIC_ORBIT_HYPERBOLA) {
  }

  /* Final conversion */
  rv[0] = matrix[0] * r1 + matrix[3] * r2;
  rv[1] = matrix[1] * r1 + matrix[4] * r2;
  rv[2] = matrix[2] * r1 + matrix[5] * r2;
  rv[3] = matrix[0] * v1 + matrix[3] * v2;
  rv[4] = matrix[1] * v1 + matrix[4] * v2;
  rv[5] = matrix[2] * v1 + matrix[5] * v2;

  return 0;
}

/**
 * @fn int orbit_rv2el(double gm, double el[], double rv[])
 *
 * @brief
 * Converts cartesian rv to orbital elements
 *
 * @return
 * Should return 0 on success, errcode otherwise
 */
int orbit_rv2el(double gm, double el[], double rv[]){

  double h[3], h2, s2h;
  double r2, s2r, v2, s2v, energy;
  double rdotv, rdot, E, sin_E, cos_E, sin_w, cos_w;
  double unit, divider, u, w, test;
  int orbit_type;

  unit = 1.0;

  /* Angular momentum vector per unit mass (h) */
  h[0] = rv[1] * rv[5] - rv[2] * rv[4];
  h[1] = rv[2] * rv[3] - rv[0] * rv[5];
  h[2] = rv[0] * rv[4] - rv[1] * rv[3];

  h2 = h[0] * h[0] + h[1] * h[1] + h[2] * h[2];
  s2h = sqrt(h2);

  /* Energy */
  r2 = rv[0] * rv[0] + rv[1] * rv[1] + rv[2] * rv[2];
  s2r = sqrt(r2);

  v2 = rv[3] * rv[3] + rv[4] * rv[4] + rv[5] * rv[5];
  s2v = sqrt(v2);

  energy = 0.5 * v2 - gm / s2r;

  /* Choose orbit type according to energy */
  if (energy < 0.0) orbit_type = MECHANIC_ORBIT_ELLIPSE;
  if (energy > 0.0) {
    orbit_type = MECHANIC_ORBIT_HYPERBOLA;
    test = fabs(energy * s2r / gm);
    if (test < sqrt(TINY)) orbit_type = MECHANIC_ORBIT_PARABOLA;
  }

  if (orbit_type == MECHANIC_ORBIT_ELLIPSE) {

    /* Semi major axis (a) */
    el[0] = - 0.5 * gm / energy;

    /* Eccentricity (e) */
    el[1] = sqrt(unit - h2 / (gm * el[0]));

    /* Inclination (I) */
    el[2] = acos(h[2] / s2h);

    /* Eccentric anomaly (E) */
    rdotv = rv[0] * rv[3] + rv[1] * rv[4] + rv[2] * rv[5];
    rdot = rdotv / s2r;

    sin_E = s2r * rdot / (el[1] * sqrt(gm * el[0]));
    cos_E = (el[0] - s2r) / (el[0] * el[1]);

    E = acos(cos_E);
    if (rdotv < 0.0) E = 2.0 * PI - E;

    /* Right Ascension of the Ascending Node (OMEGA) */
    el[3] = atan2(h[0], -h[1]);
    if (el[3] < 0.0) el[3] = 2.0 * PI + el[3];

    /* Argument of pergiee (omega) */
    divider = unit - el[1] * cos_E;
    sin_w = sqrt(unit - pow(el[1], 2.0)) * sin_E / divider;
    cos_w = (cos_E - el[1]) / divider;

    w = atan2(sin_w, cos_w);
    if (w < 0.0) w = w + 2.0 * PI;

    /* Argument of latitude (u) */
    u = atan2(rv[2] / sin(el[2]), rv[0] * cos(el[3]) + rv[1] * sin(el[3]));
    if (u < 0.0) u = u + 2.0 * PI;

    el[4] = u - w;
    if (el[4] < 0.0) el[4] = el[4] + 2.0 * PI;

    /* Mean anomaly (M) */
    el[5] = E - el[1] * sin_E;
  }

  if (orbit_type == MECHANIC_ORBIT_PARABOLA) {
  }

  if (orbit_type == MECHANIC_ORBIT_HYPERBOLA) {
  }

  return 0;
}

int orbit_rv2el_ellipse(double gm, double el[], double rv[]){
  return 0;
}

int orbit_rv2el_parabola(double gm, double el[], double rv[]){
  return 0;
}

int orbit_rv2el_hyperbola(double gm, double el[], double rv[]){
  return 0;
}

/**
 * @fn in orbit_kepler2cart(int direction, int precision, int ot, double gm,
 * double el[], double rv[])
 *
 * @brief Wrapper function for converting orbital elements to cartesian rv
 * frame in both directions
 *
 * @return
 * Should return 0 on success, errcode otherwise
 */
int orbit_kepler2cart(int direction, int precision,
    double gm, double el[], double rv[]){

  int mstat;

  if (direction == -1) {
    mstat = orbit_rv2el(gm, el, rv);
  } else {
    mstat = orbit_el2rv(precision, gm, el, rv);
  }

  return mstat;
}

/**
 * mass -- the mass of the body
 * rv[] -- barycentric rv
 * rp -- barycentric rp
 */
int orbit_baryrv2baryrp(int direction, double mass, double rv[], double rp[]){

  int mstat = 0, i;

  if (direction == -1) {
    for (i = 3; i < 6; i++) rv[i] = rp[i] / mass;
  } else {
    for (i = 3; i < 6; i++) rp[i] = mass * rv[i];
  }

  return mstat;
}

/**
 * Author: K. Gozdziewski (2001)
 *
 * mass[0] -- the central body mass
 * mass[1] -- sum of the masses
 * mass[2] -- the mass of the body
 *
 * hrv[] -- heliocentric rv
 * brv[] -- barycentric rv
 */
int orbit_helio2bary(int direction, double mass[], double hrv[], double brv[]){

  int mstat = 0;

  return mstat;
}

/**
 * Author: K. Gozdziewski (2001)
 */
int orbit_helio2poincare(int direction, double mass[], double hrv[], double prv[]){

  int mstat = 0;

  return mstat;
}

/**
 * @fn double rad2deg(double angle)
 *
 * @brief Converts radians to degress
 *
 * @param angle
 *  An angle in radians
 *
 * @return
 *  An angle in degrees
 */
double rad2deg(double angle){

  return angle * 180.0 / PI;
}

/**
 * @fn double deg2rad(double angle)
 *
 * @brief Converts degrees to radians
 *
 * @param angle
 *  An angle in degrees
 *
 * @return
 *  An angle in degrees
 */
double deg2rad(double angle){

  return angle * PI / 180;
}

