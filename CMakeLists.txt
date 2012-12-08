cmake_minimum_required (VERSION 2.8)
project (mechanic)

set (Mechanic_VERSION_MAJOR 2)
set (Mechanic_VERSION_MINOR 2)
set (Mechanic_VERSION_PATCH 0)
set (Mechanic_VERSION_API 0220)

set (package_version
  "\"${Mechanic_VERSION_MAJOR}.${Mechanic_VERSION_MINOR}.${Mechanic_VERSION_PATCH}\"")

set (package_name "\"${CMAKE_PROJECT_NAME}\"")
set (package_author "\"Celestial Mechanics Group (TCFA/NCU)\"")
set (package_bugs "\"mechanics@astri.umk.pl\"")
set (package_url "\"http://git.ca.umk.pl\"")
set (package_version_major "\"${Mechanic_VERSION_MAJOR}\"")
set (package_version_minor "\"${Mechanic_VERSION_MINOR}\"")
set (package_version_patch "\"${Mechanic_VERSION_PATCH}\"")
set (package_version_api "\"${Mechanic_VERSION_API}\"")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (platform_darwin 1)
  set (platform_linux 0)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (platform_darwin 0)
  set (platform_linux 1)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckCSourceCompiles)
include (CheckCCompilerFlag)

add_definitions (-DHAVE_CONFIG_H)

check_c_compiler_flag(-Wall HAS_WALL)
check_c_compiler_flag(-std=c99 HAS_STD_C99)

if (HAS_WALL)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif (HAS_WALL)

if (HAS_STD_C99)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif (HAS_STD_C99)
  
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES (strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (sys/dir.h HAVE_DIR_H)
CHECK_INCLUDE_FILES (sys/stat.h HAVE_STAT_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_TYPES_H)
CHECK_INCLUDE_FILES (sys/wait.h HAVE_WAIT_H)
CHECK_INCLUDE_FILES (sys/param.h HAVE_PARAM_H)
CHECK_INCLUDE_FILES (signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILES (fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILES (dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILES (time.h HAVE_TIME_H)
CHECK_INCLUDE_FILES (ctype.h HAVE_CTYPE_H)

CHECK_INCLUDE_FILES (dlfcn.h HAVE_DLFCN_H)
CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_DLFCN_LIB)
if (NOT HAVE_DLFCN_H OR NOT HAVE_DLFCN_LIB)
  MESSAGE(FATAL_ERROR "No dlopen() found in your system.")
endif (NOT HAVE_DLFCN_H OR NOT HAVE_DLFCN_LIB)

CHECK_INCLUDE_FILES (math.h HAVE_MATH_H)
CHECK_LIBRARY_EXISTS(m pow "" HAVE_M_LIB)
if (NOT HAVE_MATH_H OR NOT HAVE_M_LIB)
  MESSAGE(FATAL_ERROR "Math library is required")
endif (NOT HAVE_MATH_H OR NOT HAVE_M_LIB)

CHECK_INCLUDE_FILES (popt.h HAVE_POPT_H)
CHECK_LIBRARY_EXISTS(popt poptGetContext "" HAVE_POPT_LIB)
if (NOT HAVE_POPT_H OR NOT HAVE_POPT_LIB)
  MESSAGE(FATAL_ERROR "Popt library is required")
endif (NOT HAVE_POPT_H OR NOT HAVE_POPT_LIB)

CHECK_INCLUDE_FILES (mpi.h HAVE_MPI_H)
CHECK_LIBRARY_EXISTS(mpi MPI_Bcast "" HAVE_MPI_LIB)
if (NOT HAVE_MPI_H OR NOT HAVE_MPI_LIB)
  MESSAGE(FATAL_ERROR "MPI2 not found. Please check the MPI installation.")
endif (NOT HAVE_MPI_H OR NOT HAVE_MPI_LIB)

CHECK_INCLUDE_FILES (hdf5.h HAVE_HDF5_H)
CHECK_LIBRARY_EXISTS(hdf5 H5Dopen2 "" HAVE_HDF5_LIB)
if (NOT HAVE_HDF5_H OR NOT HAVE_HDF5_LIB)
  MESSAGE(FATAL_ERROR "HDF5 not found. Please check the HDF5 installation.")
endif (NOT HAVE_HDF5_H OR NOT HAVE_HDF5_LIB)

CHECK_INCLUDE_FILES ("hdf5.h;hdf5_hl.h" HAVE_HDF5_HL_H)
CHECK_LIBRARY_EXISTS(hdf5_hl H5LTset_attribute_float "" HAVE_HDF5_HL_LIB)
if (NOT HAVE_HDF5_HL_H OR NOT HAVE_HDF5_HL_LIB)
  MESSAGE(FATAL_ERROR "HDF5 HL not found. Please recompile the HDF5 with HL support.")
endif (NOT HAVE_HDF5_HL_H OR NOT HAVE_HDF5_HL_LIB)

CONFIGURE_FILE (
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/src/config.h
)

add_subdirectory(src)
add_subdirectory(src/core)
add_subdirectory(src/modes)
add_subdirectory(src/modules)
add_subdirectory(examples)

SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mechanic")
SET (CPACK_PACKAGE_VENDOR "Celestial Mechanics Group (TCFA/NCU)")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
SET (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET (CPACK_PACKAGE_VERSION_MAJOR "${Mechanic_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR "${Mechanic_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH "${Mechanic_VERSION_PATCH}")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "/usr/local")
SET (CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${Mechanic_VERSION_MAJOR}.${Mechanic_VERSION_MINOR}.${Mechanic_VERSION_PATCH}")
SET (CPACK_SOURCE_IGNORE_FILES
  "/build/;/.DS_Store;/.gitignore;/.git/;~$;*.so;*.dylib;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

