/**
 * Simple parallel-GA implementation for the Mechanic
 * ==================================================
 *
 * The algorithm is based on the geneticAlgorithm1.c by John LeFlohic (February 24, 1999)
 * http://www-cs-students.stanford.edu/~jl/Essays/ga.html
 *
 * In this example each generation is a new task pool (see PoolProcess())
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_ga.c -o libmechanic_module_ex_ga.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_ga -x 10 -y 10
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 * 
 * Note
 * ----
 *
 * The result of the GA depends on the number of organisms. For a good test try:
 *
 *    mpirun -np 4 mechanic -p ex_ga -x 10 -y 10
 *
 * which will use 100 organisms and should result with:
 *
 *    Generation 42 with fitness 1681
 *
 * The maximum number of generations is arbitrary, and i.e. for default 5x5 task board
 * this limit will be reached (in fact, the original code ends with over 10000
 * generations).
 *
 * The maximum number of generations clearly depends on the available memory (number of
 * task pools that can be allocated on each node).
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mechanic.h"

#define MAX_GENERATIONS 1000

/**
 * GA functions prototypes
 */
int InitializeOrganisms(pool *p, int **population, int **model, int genes, int alleles);
int ProduceNextGeneration(pool *p, int genes, int alleles, double mutation_rate);
int EvaluateOrganisms(pool *p, int max_fitness);
int Fitness(pool *p, task *t, int **population, int **model, int genes);
int SelectOneOrganism(pool *p);

/**
 * Implements Init()
 *
 * We overwrite here the default pools number
 */
int Init(init *i) {
  i->pools = MAX_GENERATIONS;
  return SUCCESS;
}

/**
 * Implements Setup()
 */
int Setup(setup *s) {
  s->options[0] = (options) {
    .space="ga", .name="max-generations", .shortName='\0',
    .value="1000", .type=C_INT, .description="Maximum generations (max 1000)"};
  s->options[1] = (options) {
    .space="ga", .name="genes", .shortName='\0',
    .value="20", .type=C_INT, .description="Number of genes"};
  s->options[2] = (options) {
    .space="ga", .name="alleles", .shortName='\0',
    .value="4", .type=C_INT, .description="Number of types of genes"};
  s->options[3] = (options) {
    .space="ga", .name="mutation-rate", .shortName='\0',
    .value="0.001", .type=C_DOUBLE, .description="Mutation rate"};
  s->options[4] = (options) {
    .space="ga", .name="max-fitness", .shortName='\0',
    .value="20", .type=C_INT, .description="Maximum fitness"};
  s->options[5] = (options) OPTIONS_END;

  return SUCCESS;
}

/**
 * Implements Storage()
 */
int Storage(pool *p, void *s) {
  int pool_size;
  int genes;

  pool_size = p->board->layout.dims[0] * p->board->layout.dims[1];
  MReadOption(p, "genes", &genes);

  /**
   * Path: /Pools/pool-ID/population
   * Current population
   */
  p->storage[0].layout = (schema) {
    .name = "population",
    .rank = 2,
    .dims[0] = pool_size,
    .dims[1] = genes,
    .use_hdf = 1,
    .sync = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT,
  };

  /**
   * Path: /Pools/pool-ID/model
   * The model organism
   */
  p->storage[1].layout = (schema) {
    .name = "model",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = genes,
    .use_hdf = 1,
    .sync = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT,
  };

  /**
   * Path: /Pools/pool-ID/tmp-data
   * Temporary space for the pool data, not stored in the master datafile
   */
  p->storage[2].layout = (schema) {
    .name = "tmp-data",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = genes,
    .use_hdf = 0,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT,
  };

  /**
   * Path: /Pools/pool-ID/children
   * The children of the current population
   */
  p->storage[3].layout = (schema) {
    .name = "children",
    .rank = 2,
    .dims[0] = pool_size,
    .dims[1] = genes,
    .use_hdf = 0,
    .sync = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT,
  };

  /**
   * Path: /Pools/pool-ID/Tasks/fitness
   * The fitness of the each organism of the population
   * (How far we are from the model one)
   */
  p->task->storage[0].layout = (schema) {
    .name = "fitness",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = 1, // 0 - fitness
    .use_hdf = 1,
    .sync = 1,
    .storage_type = STORAGE_LIST,
    .datatype = H5T_NATIVE_INT,
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 *
 * If p->pid = 0 we initialize the population (the first generation), otherwise, we copy
 * the children of the previous generation to the population dataset. The previous
 * population is accessed through the **all pointer.
 */
int PoolPrepare(pool **all, pool *p, void *s) {
  int i, j, genes, alleles;
  int **current, **model;

  MAllocate2(p, "children", current, int);
  MAllocate2(p, "model", model, int);

  MReadOption(p, "genes", &genes);
  MReadOption(p, "alleles", &alleles);

  if (p->rid == 0) {
    if (p->pid == 0) {
      InitializeOrganisms(p, current, model, genes, alleles);
      MWriteData(p, "population", &current[0][0]);
      MWriteData(p, "model", &model[0][0]);
    } else {
      MReadData(all[p->pid-1], "children", &current[0][0]);
      MWriteData(p, "population", &current[0][0]);
      
      /* Copy the model */
      MReadData(all[p->pid-1], "model", &model[0][0]);
      MWriteData(p, "model", &model[0][0]);
    }
  }

  free(current);
  free(model);

  return SUCCESS;
}

/**
 * @brief Implements TaskProcess()
 *
 * We compute here the fitness function (in parallel, each worker receives an organism to
 * check). We return here only the fitness of the specified organism.
 */
int TaskProcess(pool *p, task *t, void *s) {
  int genes;
  int **fitness, **population, **model;

  MAllocate2(t, "fitness", fitness, int);
  MAllocate2(p, "population", population, int);
  MAllocate2(p, "model", model, int);

  MReadOption(p, "genes", &genes);

  MReadData(p, "population", &population[0][0]);
  MReadData(p, "model", &model[0][0]);

  fitness[0][0] = Fitness(p, t, population, model, genes);

  MWriteData(t, "fitness", &fitness[0][0]);

  free(population); free(model);
  free(fitness);

  return TASK_FINALIZE;
}

/**
 * Implements PoolProcess()
 *
 * We decide here, whether the model has been reached or not. When the model is not
 * reached, we generate the next population (children of the current population).
 */
int PoolProcess(pool **all, pool *p, void *s) {
  int perfectGeneration = 0;
  int maxgen, max_fitness, genes, alleles;
  double mutation_rate;
  int **fitness;

  MAllocate2(p, "tmp-data", fitness, int);

  MReadOption(p, "max-generations", &maxgen);
  MReadOption(p, "max-fitness", &max_fitness);
  MReadOption(p, "genes", &genes);
  MReadOption(p, "alleles", &alleles);
  MReadOption(p, "mutation-rate", &mutation_rate);

  /* Has the model organism been reached? */
  perfectGeneration = EvaluateOrganisms(p, max_fitness);

  MReadData(p, "tmp-data", &fitness[0][0]);
  if (perfectGeneration) {
    Message(MESSAGE_OUTPUT, "Generation %d with fitness %d\n", p->pid+1, fitness[0][0]);
    free(fitness);
    return POOL_FINALIZE;
  }

  if (p->pid+1 >= maxgen || p->pid+1 >= MAX_GENERATIONS) {
    Message(MESSAGE_WARN, "Max generations (%d) reached.\n", p->pid+1);
    free(fitness);
    return POOL_FINALIZE;
  }

  /* The model organism has not been reached. We generate the next population */
  ProduceNextGeneration(p, genes, alleles, mutation_rate);
  free(fitness);

  return POOL_CREATE_NEW;
}

/**
 * geneticAlgorithm1.c based functions
 */

/**
 * Initialize the first generation
 */
int InitializeOrganisms(pool *p, int **population, int **model, int genes, int alleles) {
  int i = 0, j = 0;

  /* Organisms */
  for (i = 0; i < p->pool_size; i++) {
    for (j = 0; j < genes; j++) {
      population[i][j] = rand() % alleles;
    }
  }

  /* Model */
  for (i = 0; i < genes; i++) {
    model[0][i] = rand() % alleles;
  }

  return 0;
}

/**
 * Compute the fitness of current organism (task)
 */
int Fitness(pool *p, task *t, int **population, int **model, int genes) {
  int i = 0, fit = 0;

  for (i = 0; i < genes; i++) {
    if (population[t->tid][i] == model[0][i]) {
      fit++;
    }
  }

  return fit;
}

/**
 * Evaluate organisms in the current generation (pool)
 */
int EvaluateOrganisms(pool *p, int max_fitness) {
  int i;
  int **tmp, **fitness;

  MAllocate2(p, "tmp-data", tmp, int);
  MAllocate2(p->task, "fitness", fitness, int);

  MReadData(p->task, "fitness", &fitness[0][0]);

  tmp[0][0] = 0;

  /* Total fitness */
  for (i = 0; i < p->pool_size; i++) {
    tmp[0][0] += fitness[i][0];
  }
  MWriteData(p, "tmp-data", &tmp[0][0]);

  for (i = 0; i < p->pool_size; i++) {
    if (fitness[i][0] == max_fitness) {
      free(tmp); free(fitness);
      return 1;
    }
  }

  free(tmp); free(fitness);
  return 0;
}

/**
 * Produce the next generation
 */
int ProduceNextGeneration(pool *p, int genes, int alleles, double mutation_rate) {
  int i, j;
  int parentOne;
  int parentTwo;
  int crossoverPoint;
  int mutateThisGene;
  int **population, **children;

  MAllocate2(p, "population", population, int);
  MAllocate2(p, "children", children, int);

  MReadData(p, "population", &population[0][0]);

  for (i = 0; i < p->pool_size; i++) {
    parentOne = SelectOneOrganism(p);
    parentTwo = SelectOneOrganism(p);
    crossoverPoint = rand() % genes;

    for (j = 0; j < genes; j++) {
      mutateThisGene = rand() % (int)(1.0/mutation_rate);
      if (mutateThisGene == 0) {
        children[i][j] = rand() % alleles;
      } else {
        if (j < crossoverPoint) {
          children[i][j] = population[parentOne][j];
        } else {
          children[i][j] = population[parentTwo][j];
        }
      }
    }
  }

  MWriteData(p, "children", &children[0][0]);

  free(population); free(children);

  return 0;
}

/**
 * Selects the parent
 */
int SelectOneOrganism(pool *p) {
  int i = 0, t = 0, r = 0;
  int **tmp, **fitness;

  MAllocate2(p->task, "fitness", fitness, int);
  MAllocate2(p, "tmp-data", tmp, int);

  MReadData(p->task, "fitness", &fitness[0][0]);
  MReadData(p, "tmp-data", &tmp[0][0]);

  r = rand() % (tmp[0][0] + 1);

  for (i = 0; i < p->pool_size; i++) {
    t += fitness[i][0];
    if (t >= r) {
      free(tmp); free(fitness);
      return i;
    }
  }

  free(tmp); free(fitness);
  return 0;
}

