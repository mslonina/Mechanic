Random Number Generation - Multiple Streams
===========================================

Authors
-------

Steve Park & Dave Geyer

- RNGS Latest Revision: 09-22-98
- RVGS Latest Revision: 10-28-98


Random Number Generators
------------------------

This is an ANSI C library for multi-stream random number generation.  
The use of this library is recommended as a replacement for the ANSI C 
rand() and srand() functions, particularly in simulation applications 
where the statistical 'goodness' of the random number generator is 
important.  The library supplies 256 streams of random numbers; use 
SelectStream(s) to switch between streams indexed s = 0,1,...,255.

The streams must be initialized.  The recommended way to do this is by
using the function PlantSeeds(x) with the value of x used to initialize 
the default stream and all other streams initialized automatically with
values dependent on the value of x.  The following convention is used 
to initialize the default stream:

- if x > 0 then x is the state
- if x < 0 then the state is obtained from the system clock
- if x = 0 then the state is to be supplied interactively.

The generator used in this library is a so-called 'Lehmer random number
generator' which returns a pseudo-random number uniformly distributed
0.0 and 1.0.  The period is (m - 1) where m = 2,147,483,647 and the
smallest and largest possible values are (1 / m) and 1 - (1 / m)
respectively.  For more details see:
 
"Random Number Generators: Good Ones Are Hard To Find", Steve Park and Keith Miller,
Communications of the ACM, October 1988

Random Variate Generators
-------------------------

This is an ANSI C library for generating random variates from six discrete 
distributions

     Generator         Range (x)     Mean         Variance

     Bernoulli(p)      x = 0,1       p            p*(1-p)
     Binomial(n, p)    x = 0,...,n   n*p          n*p*(1-p)
     Equilikely(a, b)  x = a,...,b   (a+b)/2      ((b-a+1)*(b-a+1)-1)/12
     Geometric(p)      x = 0,...     p/(1-p)      p/((1-p)*(1-p))
     Pascal(n, p)      x = 0,...     n*p/(1-p)    n*p/((1-p)*(1-p))
     Poisson(m)        x = 0,...     m            m
 
and seven continuous distributions

     Uniform(a, b)     a < x < b     (a + b)/2    (b - a)*(b - a)/12 
     Exponential(m)    x > 0         m            m*m
     Erlang(n, b)      x > 0         n*b          n*b*b
     Normal(m, s)      all x         m            s*s
     Lognormal(a, b)   x > 0            see below
     Chisquare(n)      x > 0         n            2*n 
     Student(n)        all x         0  (n > 1)   n/(n - 2)   (n > 2)

For the a Lognormal(a, b) random variable, the mean and variance are

     mean = exp(a + 0.5*b*b)
     variance = (exp(b*b) - 1) * exp(2*a + b*b)

