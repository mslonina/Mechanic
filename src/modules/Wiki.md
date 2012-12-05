/**
 * @page pool_loop The pool loop explained
 *
 * After the core and the module are bootstrapped, the Mechanic enters the four-step pool
 * loop:
 *
 * 1. PoolPrepare()
 *  All nodes enter the PoolPrepare(). Data of all previous pools is passed to this
 *  function, so that we may use them to prepare data for the current pool. The current pool
 *  data is broadcasted to all nodes and stored in the master datafile.
 *
 * 2. The Task Loop
 *  All nodes enter the task loop. The master node prepares the task during
 *  TaskPrepare(). Each worker receives a task, and calls the TaskProcess(). The task
 *  data, marked with use_hdf = 1 are received by the master node after the
 *  TaskProcess() is finished and stored during the CheckpointProcess().
 *
 * 3. The Checkpoint
 *  The CheckpointPrepare() function might be used to adjust the received data (2D array
 *  of packed data, each row is a separate task). The pool data might be adjusted here
 *  as well, the data is stored again during CheckpointProcess().
 *
 * 4. PoolProcess()
 *  After the task loop is finished, the PoolProcess() is used to decide whether to
 *  continue the pool loop or not. The data of all pools is passed to this function. 
 *
 * ### Order of function hooks
 *
 *
 *     Init()
 *     Setup()
 *
 *     Prepare()
 *        [Pool loop]
 *    
 *          Storage()
 *          NodePrepare()
 *          PoolPrepare()
 *          LoopPrepare()
 *    
 *          [Task loop]
 *            TaskMapping()
 *            TaskPrepare()
 *            TaskProcess()
 *          [/Task loop]
 *    
 *          LoopProcess()
 *          PoolProcess()
 *          NodeProcess()
 *    
 *        [/Pool loop]
 *     Process()
 */


