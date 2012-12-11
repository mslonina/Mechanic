/**
 * Working with datasets
 * =====================
 *
 * In this example we manipulate the datasets with advanced hooks
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 -lhdf5 -lhdf5_hl mechanic_module_ex_dset.c -o libmechanic_module_ex_dset.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p ex_dset -x 10 -y 20
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
    .name = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 3,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
    .datatype = H5T_NATIVE_DOUBLE
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
  double buffer_one[1][3];

  // The vertical position of the pixel
  buffer_one[0][0] = t->location[0];

  // The horizontal position of the pixel
  buffer_one[0][1] = t->location[1];

  // The state of the system
  buffer_one[0][2] = t->tid;

  MWriteData(t, "result", buffer_one);
  
  return SUCCESS;
}

/**
 * Implements DatasetPrepare()
 *
 * We use this hook to write some simple attributes useful for postprocessing in i.e.
 * matplotlib.
 *
 * This hook is invoked on each dataset defined through the Storage(), right after its
 * creation. The proper HDF5 dataset object `h5dataset`, as well as the top level group `h5location`,
 * are passed, so that any HDF5 operation may be performed here.
 * You may use standard HDF5 API, as well as HDF5_HL API.
 */
int DatasetPrepare(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s) {
  double xmin, xmax, ymin, ymax, zmin, zmax;
  hsize_t adims;
  hid_t hstat;
  double attr_data[1];

  /* Check for the specific dataset */
  if (strcmp(d->layout.name, "result") == 0) {
    xmin = Option2Double("core", "xmin", s->head);
    xmax = Option2Double("core", "xmax", s->head);
    ymin = Option2Double("core", "ymin", s->head);
    ymax = Option2Double("core", "ymax", s->head);
    zmin = Option2Double("core", "zmin", s->head);
    zmax = Option2Double("core", "zmax", s->head);

    adims = 1;
    attr_data[0] = xmin;
    hstat = H5LTset_attribute_double(h5location, d->layout.name, "xmin", attr_data, adims);
    
    attr_data[0] = xmax;
    hstat = H5LTset_attribute_double(h5location, d->layout.name, "xmax", attr_data, adims);
    
    attr_data[0] = ymin;
    hstat = H5LTset_attribute_double(h5location, d->layout.name, "ymin", attr_data, adims);
    
    attr_data[0] = ymax;
    hstat = H5LTset_attribute_double(h5location, d->layout.name, "ymax", attr_data, adims);
    
    attr_data[0] = zmin;
    hstat = H5LTset_attribute_double(h5location, d->layout.name, "zmin", attr_data, adims);
    
    attr_data[0] = zmax;
    hstat = H5LTset_attribute_double(h5location, d->layout.name, "zmax", attr_data, adims);
  }

  return SUCCESS;
}

/**
 * Implements DatasetProcess()
 *
 * This hook is invoked after the data is stored. The valid HDF5 dataset object `h5dataset`,
 * as well as the parent group `h5location` are passed, so that
 * you can operate on the entire dataset with HDF5 API.
 */
int DatasetProcess(hid_t h5location, hid_t h5dataset, pool *p, storage *d, setup *s) {
  Message(MESSAGE_INFO, "Dataset name: %s\n", d->layout.name);
  return SUCCESS;
}
