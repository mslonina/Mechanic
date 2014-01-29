/**
 * Compound datatypes
 * ==================
 *
 * In this example, we use H5T_COMPOUND data type for use with structures
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_compound.c -o libmechanic_module_ex_compound.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_compound -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 */
#include "mechanic.h"

typedef struct {
  int id;
  double temperature;
  double pressure;
} sensor_t;

typedef struct {
  long id;
  double position[3];
  double velocity[3];
  double heat_map[4][5][6];
  double geo_data[4][5][6];
} particle_t;

/**
 * Implements Init()
 */
int Init(init *i) {
  i->banks_per_pool = 8;
  i->banks_per_task = 8;
  i->min_cpu_required = 1;
  return SUCCESS;
}

/**
 * Implements Storage()
 */
int Storage(pool *p) {
  p->storage[0].layout = (schema) {
    .name = "pool-sensors",
    .rank = 2,
    .dims[0] = 5,
    .dims[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT,
    .compound_size = sizeof(sensor_t),
  };

  p->storage[0].layout->fields[0] = 
    (fields_type) {
      .name = "id", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, id)
    };

  p->storage[0].layout->fields[1] = 
    (fields_type) {.name = "temperature", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, temperature)};

  p->storage[0].layout->fields[2] = 
    (fields_type) {.name = "pressure", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, pressure)};

  return SUCCESS;
  /*
  *p->storage[1].layout = (schema) {
    .name = "pool-particles",
    .rank = 2,
    .dims[0] = 5,
    .dims[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_COMPOUND,
    .compound_size = sizeof(particle_t)
  };

  p->storage[1].layout->fields[0] = 
    (fields_type) {.name = "id", .datatype = H5T_NATIVE_LONG, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(particle_t, id)};

  p->storage[1].layout->fields[1] = 
    (fields_type) {.name = "position", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .offset = HOFFSET(particle_t, position)};

  p->storage[1].layout->fields[2] = 
    (fields_type) {.name = "velocity", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .offset = HOFFSET(particle_t, velocity)};
 
  p->storage[1].layout->fields[3] = 
    (fields_type) {.name = "heat-map", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .offset = HOFFSET(particle_t, heat_map)};
 
  p->storage[1].layout->fields[4] = 
    (fields_type) {.name = "geo-data", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .offset = HOFFSET(particle_t, geo_data)};
 
  *p->task->storage[0].layout = (schema) {
    .name = "sensors",
    .rank = 2,
    .dims[0] = 5,
    .dims[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_COMPOUND,
    .compound_size = sizeof(sensor_t)
  };
  
  p->task->storage[0].layout->fields[0] = 
    (fields_type) {.name = "id", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, id)};

  p->task->storage[0].layout->fields[1] = 
    (fields_type) {.name = "temperature", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, temperature)};

  p->task->storage[0].layout->fields[2] = 
    (fields_type) {.name = "pressure", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, pressure)};
 
  *p->task->storage[1].layout = (schema) {
    .name = "sensors-board",
    .rank = TASK_BOARD_RANK,
    .dims[0] = 5,
    .dims[1] = 5,
    .dims[2] = 1,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_TEXTURE,
    .datatype = H5T_COMPOUND,
    .compound_size = sizeof(sensor_t)
  };
  
  p->task->storage[1].layout->fields[0] = 
    (fields_type) {.name = "id", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, id)};

  p->task->storage[1].layout->fields[1] = 
    (fields_type) {.name = "temperature", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, temperature)};

  p->task->storage[1].layout->fields[2] = 
    (fields_type) {.name = "pressure", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(sensor_t, pressure)};

  *p->task->storage[2].layout = (schema) {
    .name = "particles",
    .rank = TASK_BOARD_RANK,
    .dims[0] = 1,
    .dims[1] = 1,
    .dims[2] = 1,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_TEXTURE,
    .datatype = H5T_COMPOUND,
    .compound_size = sizeof(particle_t)
  };

  p->task->storage[2].layout->fields[0] = 
    (fields_type) {.name = "id", .datatype = H5T_NATIVE_LONG, 
      .rank = 1, .dims = {1}, .offset = HOFFSET(particle_t, id)};

  p->task->storage[2].layout->fields[1] = 
    (fields_type) {.name = "position", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .offset = HOFFSET(particle_t, position)};

  p->task->storage[2].layout->fields[2] = 
    (fields_type) {.name = "velocity", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .offset = HOFFSET(particle_t, velocity)};
 
  p->task->storage[2].layout->fields[3] = 
    (fields_type) {.name = "heat-map", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .offset = HOFFSET(particle_t, heat_map)};
 
  p->task->storage[2].layout->fields[4] = 
    (fields_type) {.name = "geo-data", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .offset = HOFFSET(particle_t, geo_data)};
 */

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **all, pool *p) {
  unsigned int i, j, dims[MAX_RANK];

  sensor_t sensors[5][5];
  sensor_t read_sensors[5][5];

  particle_t particles[5][5];
  particle_t read_particles[5][5];

  return SUCCESS;
  MGetDims(p, "pool-sensors", dims);

  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      sensors[i][j].id = i + j;
      sensors[i][j].temperature = 33.6;
      sensors[i][j].pressure = 1013.3;
      particles[i][j].position[0] = 0.1;
      particles[i][j].position[1] = 0.2;
      particles[i][j].position[2] = 0.3;
      particles[i][j].velocity[0] = 1.1;
      particles[i][j].velocity[1] = 1.2;
      particles[i][j].velocity[2] = 1.3;
    }
  }

  MWriteData(p, "pool-sensors", &sensors[0][0]);
  MReadData(p, "pool-sensors", &read_sensors[0][0]);

  /*for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      printf("sensor[%02d][%02d] id %04d = (%.5f, %.5f)\n", 
          i, j,
          read_sensors[i][j].id,
          read_sensors[i][j].temperature,
          read_sensors[i][j].pressure);
    }
  }*/

  MWriteData(p, "pool-particles", &particles[0][0]);

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **all, pool *p) {
  return POOL_FINALIZE;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t) {
  return TASK_FINALIZE;
  sensor_t sensors[5][5], pool_sensors[5][5];
  sensor_t sensors_b[5][5][1];
  particle_t particle[1][1][1];
  unsigned i, j, dims[MAX_RANK];

  MReadData(p, "pool-sensors", &pool_sensors[0][0]);
  MGetDims(p, "pool-sensors", dims);

  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      sensors[i][j].id = pool_sensors[i][j].id;
      sensors[i][j].temperature = pool_sensors[i][j].temperature;
      sensors[i][j].pressure = pool_sensors[i][j].pressure;
      sensors_b[i][j][0].id = pool_sensors[i][j].id;
      sensors_b[i][j][0].temperature = pool_sensors[i][j].temperature;
      sensors_b[i][j][0].pressure = pool_sensors[i][j].pressure;
    }
  }

  MWriteData(t, "sensors", &sensors[0][0]);
  MWriteData(t, "sensors-board", &sensors_b[0][0][0]);

  particle[0][0][0].id = t->tid;
  MWriteData(t, "particles", &particle[0][0][0]);

  return TASK_FINALIZE;
}

