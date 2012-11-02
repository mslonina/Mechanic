Mechanic 2.x examples
=====================

List of examples
----------------

- Generation of a simple map or image
- Creating a new task pool
- Different storage layout per task pool (basic)
- Different storage layout per task pool (advanced)
- Using `Prepare()` and `Process()` hooks
- Defining and using configuration options (`Init()` and `Setup()` hooks)
- Using different datatypes, reading and writing data to the memory banks
- Using datasets of different datatypes and dimensionality
- The Mandelbrot set

Compilation
-----------

    mpicc -std=c99 -fPIC -Dpic -shared -lhdf5 -lhdf5_hl -lmechanic2 -lreadconfig mechanic_module_example.c -o libmechanic_module_example.so


