/**
 * Sample map or image
 * ===================
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 -lreadconfig mechanic_module_map.c -o libmechanic_module_map.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p map -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 */
#include "Mechanic2.h"

/**
 * Implements Storage()
 *
 * Each worker will return 1x3 result array. The master node will combine the worker
 * result arrays into one dataset suitable to process with Gnuplot PM3D. The final dataset
 * will be available at /Pools/pool-0000/Tasks/result.
 *
 * By default, we have 8 memory/storage banks per each task pool + 8 memory/storage banks
 * per task available. You may change the defaults by using the Init() hook.
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .path = "input",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .sync = 0,
    .use_hdf = 0,
    .storage_type = STORAGE_PM3D,
  };

  p->task->storage[1].layout = (schema) {
    .path = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };
  return SUCCESS;
}

/**
 * Implements TaskProcess()
 *
 * For a dynamical map, we need to store the position of the point on the map and the
 * state of the dynamical system. In this example the position is represented by
 * task location, and the state of the system by the task unique identifier.
 */
int TaskProcess(pool *p, task *t, setup *s) {

  // The vertical position of the pixel
  t->storage[1].data[0][0] = t->location[0];

  // The horizontal position of the pixel
  t->storage[1].data[0][1] = t->location[1];

  // The state of the system
  t->storage[1].data[0][2] = t->tid;

  return SUCCESS;
}

/**
 * Implements DatasetPrepare()
 *
 * We use this hook to write some simple attributes useful for postprocessing in i.e.
 * matplotlib. You may use here standard HDF5 API, as well as HDF5_HL API.
 */
int DatasetPrepare(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s) {
  double xmin, xmax, ymin, ymax;
  hsize_t adims;
  hid_t hstat;
  double attr_data[1];

  if (strcmp(d->layout.path, "result") == 0) {
    xmin = LRC_option2double("core", "xmin", s->head);
    xmax = LRC_option2double("core", "xmax", s->head);
    ymin = LRC_option2double("core", "ymin", s->head);
    ymax = LRC_option2double("core", "ymax", s->head);

    adims = 1;
    attr_data[0] = xmin;
    hstat = H5LTset_attribute_double(h5location, d->layout.path, "xmin", attr_data, adims);
    
    attr_data[0] = xmax;
    hstat = H5LTset_attribute_double(h5location, d->layout.path, "xmax", attr_data, adims);
    
    attr_data[0] = ymin;
    hstat = H5LTset_attribute_double(h5location, d->layout.path, "ymin", attr_data, adims);
    
    attr_data[0] = ymax;
    hstat = H5LTset_attribute_double(h5location, d->layout.path, "ymax", attr_data, adims);
  }

  return SUCCESS;
}
