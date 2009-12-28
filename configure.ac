AC_INIT([Mpifarm], [1.0], [slonina@astri.umk.pl])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
LT_INIT
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT(`pwd`/build)

save_CC=$CC
AX_MPI([USE_MPI="yes"],[USE_MPI="no"])
AS_IF([test AS_VAR_GET([USE_MPI]) != no],[
   CC=$MPICC
   LIBS="$MPILIBS $LIBS"
   ],[
   CC=$save_CC
   LIBS=$save_LIBS
])

AC_SUBST(USE_MPI) 
AC_CHECK_LIB(m, pow)
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(popt, poptGetNextOpt)
AC_CHECK_LIB(hdf5, H5open)
AC_OUTPUT(Makefile src/Makefile src/readconfig/Makefile src/modules/Makefile src/modules/mytest/Makefile src/modules/default/Makefile)
