# cannot use h5diff since it does not exclude attributes
execute_process(COMMAND h5dump -m "%.16f" -w 1024 -g /Pools/last/Tasks -o a.txt tex_mandelbrot-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)
execute_process(COMMAND h5dump -m "%.16f" -w 1024 -g /Pools/last/Tasks -o r.txt references/tex_mandelbrot-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

