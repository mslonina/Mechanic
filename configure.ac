AC_INIT([Mechanic], [0.12-UNSTABLE-2], [mariusz.slonina@gmail.com],
        [],[http://mechanics.astri.umk.pl/projects/mechanic])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_F77
LT_INIT
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT(`pwd`/build)

AC_DEFINE([PACKAGE_URL], ["http://mechanics.astri.umk.pl/projects/mechanic"], [Define package url])

save_CC=$CC
AX_MPI([USE_MPI="yes"],[USE_MPI="no"])
AS_IF([test AS_VAR_GET([USE_MPI]) != no],[
   CC=$MPICC
   LIBS="$MPILIBS $LIBS"
   ],[
   CC=$save_CC
   LIBS=$save_LIBS
])

AC_DEFINE([HAVE_MPI_SUPPORT], 1, [Define mpi support])
AC_SUBST(USE_MPI) 
AC_CHECK_LIB(m, pow)
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(popt, poptGetNextOpt, [], [
  echo
  echo "---------------------------------"
  echo "Popt was not found on your system"
  echo "---------------------------------"
  exit -1
])

AC_CHECK_LIB(hdf5, H5Dopen2, [], [
  echo
  echo "-----------------------------------"
  echo "HDF1.8 was not found on your system"
  echo "-----------------------------------"
  exit -1
])

AC_CHECK_LIB(readconfig, LRC_parseConfigFile, [], [
  echo
  echo "---------------------------------"
  echo "You need LibReadConfig installed."
  echo "---------------------------------"
  exit -1
])

AC_CHECK_LIB(readconfig, LRC_hdfParser, [], [
  echo
  echo "---------------------------------------------------"
  echo "LibReadConfig has been builded without HDF support."
  echo "You need LibReadConfig with HDF support."
  echo "---------------------------------------------------"
  exit -1
])

AC_OUTPUT(Makefile \
          src/Makefile \
          src/core/Makefile \
          src/modes/Makefile \
          src/modes/farm/Makefile \
          src/modes/masteralone/Makefile \
          src/modes/multifarm/Makefile \
          src/engines/Makefile \
          src/engines/gpu/Makefile \
          src/engines/odex/Makefile \
          src/engines/taylor/Makefile \
          src/modules/Makefile \
          src/modules/echo/Makefile \
          src/modules/mandelbrot/Makefile \
          src/modules/hello/Makefile \
          doc/Makefile \
          examples/Makefile \
          examples/hello/Makefile)
