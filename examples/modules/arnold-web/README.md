Mechanic module for the Arnold Web
----------------------------------

**Note: This is a part of the Mechanic package**

The implementation of the module follows the Arnold Web description from:

    Froeschle C., Guzzo M., Lega E., 
    Graphical Evolution of the Arnold Web: From Order to Chaos
    Science 289, 2108-2110, 2000

Symplectic MEGNO implementation:
 
    Gozdziewski K., Breiter S., Borczyk W.
    The long-term stability of extrasolar system HD37124. 
    Numerical study of resonance effects
    MNRAS 383, 989-999, 2008

SABA integrators family by:

    Laskar J., Robutel P.
    High order symplectic integrators for perturbed Hamiltonian systems
    CMDA 80, 39-62, 2001

If you are going to use this code, or its parts, please consider referring 
to the Authors by the following paper:

    Slonina M., Gozdziewski K., Migaszewski C.
    "Mechanic: the MPI/HDF code framework for dynamical astronomy"
    New Astronomy 2014
    http://arxiv.org/abs/1401.6344


Module compilation
------------------

    mpicc -fPIC -Dpic -shared -lmechanic -lhdf5
      mechanic_module_arnoldweb.c -o libmechanic_module_arnoldweb.so
      
 
Module options
--------------

    mpirun -np 4 mechanic -p arnoldweb --help

Options:
- step - The base time step 
- tend - the maximum period
- driver - the integrator (1 - Leapfrog, 2 - SABA3, 3 - SABA4)
- epsilon_min - the minimum perturbation parameter
- epsilon_max - the maximum perturbation parameter
- epsilon_interval - the perturbation interval step
- configuration-set - the predefined configuration to use:
 - 0: (-0.5, 1.5, -0.5, 1.5) - The Arnold Web
 - 1: (0.7, 1.3, 0.7, 1.3) - The top right zoom
 - 2: (0.318, 0.332, 0.096, 0.110)
 - 3: (0.317, 0.319, 0.096, 0.098)
 - 4: (0.950, 1.050, 0.950, 1.050)
 - 5: (0.100, 0.450, 0.100, 0.450)
 - 6: (0.250, 0.350, 0.250, 0.350)
 - 7: (0.300, 0.350, 0.150, 0.200)

- power-intervals - use power-based time intervals for time-snapshots  
- force-step - force the step size (do not use SABA_n initial time-step conversion)

Using the module
----------------

The module works both in a single dynamical map mode and the pool-based simulation
mode. If the `epsilon_max` is greater than `epsilon_min`, and the `epsilon_interval` is
specified, the module will run in the pool-based simulation, i.e.

    mpirun -np 4 mechanic -p arnoldweb --xres=512 --yres=512 \
      --epsilon-min=0.01 --epsilon-max=0.04 --epsilon-interval=0.01


Three numerical drivers are provided: the modified leapfrog, SABA3 and SABA4. To use
them, use the `driver` option:

    mpirun -np 4 mechanic -p arnoldweb --xres=512 --yres=512 \
      --driver=2 --step-size=0.3

By default, the SABA3 and SABA4 integrators convert the initial step size to:

    step = step*(pow(5,0.5) - 1.0)/2.0;

To force the step-size use `force-step` option:

    mpirun -np 4 mechanic -p arnoldweb --xres=512 --yres=512 \
      --driver=2 --step-size=0.3 --force-step

The time-snapshots are computed in the equal time intervals. To switch to power-based
time intervals, use `power-intervals` option:

    mpirun -np 4 mechanic -p arnoldweb --xres=521 --yres=512 \
      --power-intervals

Maximum intervals (time snapshots) is 10. The core `task-checkpoints` option specifies the number of
snapshots to store:

    mpirun -np 4 mechanic -p arnoldweb --xres=512 --yres=512 \
      --power-intervals --task-checkpoints=7

In case of power-based time intervals, the time-snapshots begin with `10^4`. The above
example will result with time-snapshots up to `10^(4+7) = 10^11`.

The data is stored in the `STORAGE_TEXTURE` mode and is ready for use with Matplotlib and
h5py.

