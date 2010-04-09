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

#ifndef MECHANIC_LIB_ORBIT_H
#define MECHANIC_LIB_ORBIT_H

#include <math.h>

#define MECHANIC_ORBIT_ELLIPSE 0
#define MECHANIC_ORBIT_PARABOLA 1
#define MECHANIC_ORBIT_HYPERBOLA 2

#define MECHANIC_ORBIT_DANBY 0
#define MECHANIC_ORBIT_ACCURATE 1

#define MECHANIC_ORBIT_ACCURACY_LEVEL 1.0e-15

/* CONSTANS */

/* Jupiter mass
 *
 * Units:
 * Sun mass AU**3/d**2
 * length [1AU]
 * time [1 day]
 */
#define MASSJ 0.0009547919384243
#define MUJ 0.28253421034459263e-6
#define MUS 0.2959122082855911e-3

#define PI 3.14159265358979323846	/* pi */
#define DEG2RAD PI/180.0
#define RAD2DEG 180.0/PI
#define PIBY2 M_PI_2

#define TINY 2.0e-16

#define YEAR 365.24
#define DAYINSEC 24.0*3600.0

#define VELC 299792.458
#define AU 499.00478364*VELC
#define G 6.67259e-20

#define VELCIN VELC*DAYINSEC/AU
#define VELCAU3 1.0*AU/DAYINSEC

/* Convergence criteria for Danby */
#define DANBYAC 1.0e-16
#define DANBYB 1.0e-15

/* Loop limits in the Laguerre attempts */
#define NLAG1 50
#define NLAG2 400

double orbit_kepler(int precision, double e, double m);
double orbit_kepler_iteration(int precision, double e, double m, double E);
int orbit_el2rv(int orbit_type, double gm, double el[], double rv[]);
int orbit_rv2el(int orbit_type, double gm, double el[], double rv[]);

int orbit_kepler2cart(int direction);
int orbit_baryxv2baryrp(int direction);
int orbit_helio2bary(int direction);
int orbit_helio2poincare(int direction);

double deg2rad(double angle);
double rad2deg(double angle);

#endif

