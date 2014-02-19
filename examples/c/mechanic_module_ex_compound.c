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
 * Limitations
 * -----------
 *
 * - No support for pointers (including char*)
 * - No support for nested structs
 */
#include "mechanic.h"

/**
 * Datatypes
 */
typedef struct {
  int id;
  double temperature;
  double pressure;
  int tags[35];
  char name[257];
  short points[3];
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

  // Pool bank 0
  p->storage[0].layout = (schema) {
    .name = "pool-sensors",
    .rank = 2,
    .dims[0] = 5,
    .dims[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_COMPOUND,
    .compound_size = sizeof(sensor_t),
  };

  // Field definition for the pool bank 0
  p->storage[0].field[0].layout = 
    (schema) {.name = "id", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, id)};

  p->storage[0].field[1].layout = 
    (schema) {.name = "temperature", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, temperature)};

  p->storage[0].field[2].layout = 
    (schema) {.name = "pressure", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, pressure)};
  
  p->storage[0].field[3].layout = 
    (schema) {.name = "name", .datatype = H5T_NATIVE_CHAR, 
      .rank = 1, .dims = {257}, .field_offset = HOFFSET(sensor_t, name)};

  p->storage[0].field[4].layout = 
    (schema) {.name = "tags", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {33}, .field_offset = HOFFSET(sensor_t, tags)};

  p->storage[0].field[5].layout = 
    (schema) {.name = "points", .datatype = H5T_NATIVE_SHORT, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(sensor_t, points)};

  // Pool bank 1
  p->storage[1].layout = (schema) {
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

  // Field definition for the pool bank 1
  p->storage[1].field[0].layout = 
    (schema) {.name = "id", .datatype = H5T_NATIVE_LONG, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(particle_t, id)};

  p->storage[1].field[1].layout = 
    (schema) {.name = "position", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(particle_t, position)};

  p->storage[1].field[2].layout = 
    (schema) {.name = "velocity", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(particle_t, velocity)};
 
  p->storage[1].field[3].layout = 
    (schema) {.name = "heat-map", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .field_offset = HOFFSET(particle_t, heat_map)};
 
  p->storage[1].field[4].layout = 
    (schema) {.name = "geo-data", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .field_offset = HOFFSET(particle_t, geo_data)};
 
  // Task bank 0
  p->task->storage[0].layout = (schema) {
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
  
  // Field definition for the task bank 0
  p->task->storage[0].field[0].layout = 
    (schema) {.name = "id", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, id)};

  p->task->storage[0].field[1].layout = 
    (schema) {.name = "temperature", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, temperature)};

  p->task->storage[0].field[2].layout = 
    (schema) {.name = "pressure", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, pressure)};
 
  p->task->storage[0].field[3].layout = 
    (schema) {.name = "name", .datatype = H5T_NATIVE_CHAR, 
      .rank = 1, .dims = {257}, .field_offset = HOFFSET(sensor_t, name)};

  p->task->storage[0].field[4].layout = 
    (schema) {.name = "tags", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {33}, .field_offset = HOFFSET(sensor_t, tags)};

  p->task->storage[0].field[5].layout = 
    (schema) {.name = "points", .datatype = H5T_NATIVE_SHORT, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(sensor_t, points)};

  // Task bank 1
  p->task->storage[1].layout = (schema) {
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
  
  // Field definition for the task bank 1
  p->task->storage[1].field[0].layout = 
    (schema) {.name = "id", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, id)};

  p->task->storage[1].field[1].layout = 
    (schema) {.name = "temperature", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, temperature)};

  p->task->storage[1].field[2].layout = 
    (schema) {.name = "pressure", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(sensor_t, pressure)};

  p->task->storage[1].field[3].layout = 
    (schema) {.name = "name", .datatype = H5T_NATIVE_CHAR, 
      .rank = 1, .dims = {257}, .field_offset = HOFFSET(sensor_t, name)};

  p->task->storage[1].field[4].layout = 
    (schema) {.name = "tags", .datatype = H5T_NATIVE_INT, 
      .rank = 1, .dims = {33}, .field_offset = HOFFSET(sensor_t, tags)};

  p->task->storage[1].field[5].layout = 
    (schema) {.name = "points", .datatype = H5T_NATIVE_SHORT, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(sensor_t, points)};

  // Task bank 2
  p->task->storage[2].layout = (schema) {
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

  // Field definition for the task bank 2
  p->task->storage[2].field[0].layout = 
    (schema) {.name = "id", .datatype = H5T_NATIVE_LONG, 
      .rank = 1, .dims = {1}, .field_offset = HOFFSET(particle_t, id)};

  p->task->storage[2].field[1].layout = 
    (schema) {.name = "position", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(particle_t, position)};

  p->task->storage[2].field[2].layout = 
    (schema) {.name = "velocity", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 1, .dims = {3}, .field_offset = HOFFSET(particle_t, velocity)};
 
  p->task->storage[2].field[3].layout = 
    (schema) {.name = "heat-map", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .field_offset = HOFFSET(particle_t, heat_map)};
 
  p->task->storage[2].field[4].layout = 
    (schema) {.name = "geo-data", .datatype = H5T_NATIVE_DOUBLE, 
      .rank = 3, .dims = {4,5,6}, .field_offset = HOFFSET(particle_t, geo_data)};
 
  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **all, pool *p) {
  unsigned int i, j, k, l, m, n, id = 0;
  unsigned int dims[MAX_RANK];
  sensor_t sensors[5][5];
  particle_t particles[5][5];

  MGetDims(p, "pool-sensors", dims);

  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      sensors[i][j].id = id;
      sensors[i][j].temperature = 33.6 + i - j;
      sensors[i][j].pressure = 1013.3 + i * j;
      sensors[i][j].points[0] = 1;
      sensors[i][j].points[1] = 4;
      sensors[i][j].points[2] = 3;
      
      sprintf(sensors[i][j].name, "Sensor %4d", id);
      for (k = 0; k < 33; k++) sensors[i][j].tags[k] = k;

      particles[i][j].id = id;
      particles[i][j].position[0] = 0.1 + i;
      particles[i][j].position[1] = 0.2 - j;
      particles[i][j].position[2] = 0.3 * j;
      
      particles[i][j].velocity[0] = 1.1 * i;
      particles[i][j].velocity[1] = 1.2 - j;
      particles[i][j].velocity[2] = 1.3 + i;
      
      for (l = 0; l < 4; l++) {
        for (m = 0; m < 5; m++) {
          for (n = 0; n < 6; n++) {
            particles[i][j].heat_map[l][m][n] = l * m + n;
            particles[i][j].geo_data[l][m][n] = l * m * (n + 0.023);
          }
        }
      }
      id++;
    }
  }

  MWriteData(p, "pool-sensors", &sensors[0][0]);
  MWriteData(p, "pool-particles", &particles[0][0]);

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t) {
  sensor_t sensors[5][5], pool_sensors[5][5];
  sensor_t sensors_b[5][5][1];
  particle_t particle[1][1][1];
  unsigned i, j, k, dims[MAX_RANK];

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
      
      sprintf(sensors[i][j].name, "Sensor %4d", i + j);
      sprintf(sensors_b[i][j][0].name, "Sensor %4d", i + j);

      for (k = 0; k < 3; k++) {
        sensors[i][j].points[k] = k + i - j;
        sensors_b[i][j][0].points[k] = k - i + j;
      }
      
      for (k = 0; k < 33; k++) {
        sensors[i][j].tags[k] = k*i + j;
        sensors_b[i][j][0].tags[k] = k*j - i;
      }

    }
  }

  MWriteData(t, "sensors", &sensors[0][0]);
  MWriteData(t, "sensors-board", &sensors_b[0][0][0]);

  particle[0][0][0].id = t->tid;
  particle[0][0][0].position[0] = 2.1 * t->tid;
  particle[0][0][0].position[1] = 2.2 - t->tid;
  particle[0][0][0].position[2] = 1.1 + t->tid;
  particle[0][0][0].velocity[0] = 0.1 + t->tid;
  particle[0][0][0].velocity[1] = 0.13 - t->tid;
  particle[0][0][0].velocity[2] = 2.21 * t->tid;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 5; j++) {
      for (k = 0; k < 6; k++) {
        particle[0][0][0].heat_map[i][j][k] = i + j * t->tid + k;
        particle[0][0][0].geo_data[i][j][k] = i * j * k * (t->tid + 0.023);
      }
    }
  }
  MWriteData(t, "particles", &particle[0][0][0]);

  return TASK_FINALIZE;
}

