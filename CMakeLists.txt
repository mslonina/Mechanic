cmake_minimum_required (VERSION 2.8)
project (mechanic)

set (Mechanic_VERSION_MAJOR 0)
set (Mechanic_VERSION_MINOR 12)
set (Mechanic_VERSION_PATCH 0-UNSTABLE-3-5)

set (package_version
  "\"${Mechanic_VERSION_MAJOR}.${Mechanic_VERSION_MINOR}.${Mechanic_VERSION_PATCH}\"")

set (package_name "\"${CMAKE_PROJECT_NAME}\"")
set (package_author "\"Mariusz Slonina (NCU)\"")
set (package_bugs "\"slonina@astri.umk.pl\"")
set (package_url "\"http://git.astri.umk.pl/projects/mechanic\"")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (platform_darwin 1)
  set (platform_linux 0)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (platform_darwin 0)
  set (platform_linux 1)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

option (BUILD_FORTRAN "Build Fortran bindings" on)
option (BUILD_DOCS "Build documentation" off)

if (BUILD_FORTRAN)
  enable_language (Fortran)
  set (CMAKE_Fortran_FLAGS 
    "-I. -I/usr/include -I/usr/lib -I/usr/local/include -I/usr/local/lib -std=f2003 -fbounds-check -fbacktrace -ffpe-trap=zero,overflow -fdefault-real-8")
endif (BUILD_FORTRAN)

include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckCSourceCompiles)

CHECK_INCLUDE_FILES (stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILES (dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILES (memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES (sys/stat.h HAVE_STAT_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_TYPES_H)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (popt.h HAVE_POPT_H)
CHECK_INCLUDE_FILES (math.h HAVE_MATH_H)
CHECK_INCLUDE_FILES (hdf5.h HAVE_HDF5_H)
CHECK_INCLUDE_FILES (libreadconfig.h HAVE_LRC_H)
CHECK_INCLUDE_FILES (libreadconfig_hdf5.h HAVE_LRC_HDF5_H)

CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_DLFCN_LIB)
CHECK_LIBRARY_EXISTS(hdf5 H5Dopen2 "" HAVE_HDF5_LIB)
CHECK_LIBRARY_EXISTS(m pow "" HAVE_M_LIB)
CHECK_LIBRARY_EXISTS(popt poptGetContext "" HAVE_POPT_LIB)

CHECK_LIBRARY_EXISTS(readconfig LRC_HDF5Writer "" HAVE_LRC_HDF5_LIB)

CONFIGURE_FILE (
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/src/config.h
)

add_definitions (-DHAVE_CONFIG_H)
set (CMAKE_C_FLAGS "-Wall -std=c99 -g -ggdb")

add_subdirectory(src)
add_subdirectory(src/core)
add_subdirectory(src/modes)
add_subdirectory(src/modules)

if (BUILD_FORTRAN)
  add_subdirectory(src/fortran)
endif (BUILD_FORTRAN)

SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mechanic")
SET (CPACK_PACKAGE_VENDOR "Mariusz Slonina (NCU)")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET (CPACK_PACKAGE_VERSION_MAJOR "${Mechanic_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR "${Mechanic_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH "${Mechanic_VERSION_PATCH}")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "/usr/local")
SET (CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${Mechanic_VERSION_MAJOR}.${Mechanic_VERSION_MINOR}.${Mechanic_VERSION_PATCH}-src")
SET (CPACK_SOURCE_IGNORE_FILES
  "/build/;/.DS_Store;/.gitignore;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

