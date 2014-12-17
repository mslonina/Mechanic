# http://stackoverflow.com/questions/11429055/cmake-how-create-a-single-shared-library-from-all-static-libraries-of-subprojec
include_directories(${PROJECT_SOURCE_DIR}/src/core ${PROJECT_BINARY_DIR}/src)

set (
  mainsources
  M2Main.c
  M2Main.h
  M2Core.c
  M2Core.h
)

set (
  libsources
  M2Apublic.c
  M2Cpublic.c
  M2Epublic.c
  M2Hpublic.c
  M2Mpublic.c
  M2Ppublic.c
  M2Rpublic.c
  M2Spublic.c
  M2Tpublic.c
  M2Wpublic.c
)

set (
  coresources
  M2Aprivate.c
  M2Aprivate.h
  M2Bprivate.c
  M2Bprivate.h
  M2Cprivate.c
  M2Cprivate.h
  M2CSprivate.c
  M2CSprivate.h
  M2Pprivate.c
  M2Pprivate.h
  M2Rprivate.c
  M2Rprivate.h
  M2Sprivate.c
  M2Sprivate.h
  M2Wprivate.c
  M2Wprivate.h
)

set (
  headers
  mechanic.h
  M2Main.h
  M2Apublic.h
  M2Cpublic.h
  M2Epublic.h
  M2Hpublic.h
  M2Mpublic.h
  M2Ppublic.h
  M2Rpublic.h
  M2Spublic.h
  M2Tpublic.h
  M2Wpublic.h
)

add_library(libmechanic SHARED ${libsources})
set_target_properties(libmechanic PROPERTIES OUTPUT_NAME mechanic)
target_link_libraries(libmechanic hdf5 mpi dl)

add_library(core ${coresources})
target_link_libraries(core hdf5 mpi m libmechanic dl)

add_executable(mechanic ${mainsources})
target_link_libraries(mechanic core popt mpi hdf5 libmechanic dl)

install (TARGETS libmechanic DESTINATION lib)
install (TARGETS mechanic DESTINATION bin)
install (FILES ${headers} DESTINATION include)
