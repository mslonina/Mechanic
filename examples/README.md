Mechanic 2.x examples
=====================

List of examples:
- Generation of a simple map or image
- Creating a new task pool
- Different storage layout per task pool (basic)
- Different storage layout per task pool (advanced)
- Using Prepare/Process hooks

Compilation
-----------

    mpicc -fPIC -Dpic -shared -lmechanic2 -lreadconfig mechanic_module_example.c -o libmechanic_module_example.so
