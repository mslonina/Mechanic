/* ----------------------------------------------------------------------- 
 * Name            : rngs.h  (header file for the library file rngs.c) 
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 * Latest Revision : 09-22-98
 * ----------------------------------------------------------------------- 
 */

#ifndef _RNGS_
#define _RNGS_

double Random(void);
void   PlantSeeds(long x);
void   GetSeed(long *x);
void   PutSeed(long x);
void   SelectStream(int index);
void   TestRandom(void);

long Bernoulli(double p);
long Binomial(long n, double p);
long Equilikely(long a, long b);
long Geometric(double p);
long Pascal(long n, double p);
long Poisson(double m);

double Uniform(double a, double b);
double Exponential(double m);
double Erlang(long n, double b);
double Normal(double m, double s);
double Lognormal(double a, double b);
double Chisquare(long n);
double Student(long n);

#endif

