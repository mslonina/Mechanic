cmake_minimum_required (VERSION 2.8)
project (mechanic)

set (Mechanic_VERSION_MAJOR 2)
set (Mechanic_VERSION_MINOR 3)
set (Mechanic_VERSION_PATCH 2)
set (Mechanic_VERSION_API 2.30)
set (PYTHON_PACKAGE_VERSION
  "${Mechanic_VERSION_MAJOR}.${Mechanic_VERSION_MINOR}.${Mechanic_VERSION_PATCH}")

set (package_version
  "\"${Mechanic_VERSION_MAJOR}.${Mechanic_VERSION_MINOR}.${Mechanic_VERSION_PATCH}\"")

set (package_name "\"${CMAKE_PROJECT_NAME}\"")
set (package_author "\"Celestial Mechanics Group (TCFA/NCU)\"")
set (package_bugs "\"http://github.com/mslonina/Mechanic\"")
set (package_url "\"https://github.com/mslonina/Mechanic\"")
set (package_version_major "\"${Mechanic_VERSION_MAJOR}\"")
set (package_version_minor "\"${Mechanic_VERSION_MINOR}\"")
set (package_version_patch "\"${Mechanic_VERSION_PATCH}\"")
set (package_version_api "${Mechanic_VERSION_API}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (platform_darwin 1)
  set (platform_linux 0)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (platform_darwin 0)
  set (platform_linux 1)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

OPTION (BUILD_PYTHON_TOOLKIT "Build python postprocessing toolkit" OFF)
OPTION (BUILD_VENDOR_RNGS "Build RNGS library" ON)

set (USES_MPICC 0)
if ("${CMAKE_C_COMPILER}" MATCHES "mpicc")
  set (USES_MPICC 1)
endif ("${CMAKE_C_COMPILER}" MATCHES "mpicc")

if (NOT USES_MPICC)
  MESSAGE(FATAL_ERROR "You must use MPI compiler wrapper, such as mpicc in order to compile the Mechanic")
endif (NOT USES_MPICC)

include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckCSourceCompiles)
include (CheckCCompilerFlag)

add_definitions (-DHAVE_CONFIG_H)

check_c_compiler_flag(-Wall HAS_WALL)
check_c_compiler_flag(-std=c99 HAS_STD_C99)
check_c_compiler_flag(-Wpointer-arith HAS_POINTER_ARITH)
check_c_compiler_flag(-Wmissing-prototypes HAS_MISSING_PROTOTYPES)
check_c_compiler_flag(-Wmissing-declarations HAS_MISSING_DECLARATIONS)

# GCC
#check_c_compiler_flag(-Wsign-conversion HAS_SIGN_CONVERSION)
check_c_compiler_flag(-Wpointer-sign HAS_POINTER_SIGN)

# Intel
check_c_compiler_flag(-Wsign-compare HAS_SIGN_COMPARE)

if (HAS_WALL)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -ggdb")
endif (HAS_WALL)

if (HAS_STD_C99)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif (HAS_STD_C99)

if (HAS_POINTER_ARITH)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith")
endif (HAS_POINTER_ARITH)

#if (HAS_SIGN_CONVERSION)
#  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-conversion")
#endif (HAS_SIGN_CONVERSION)
  
if (HAS_MISSING_PROTOTYPES)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes")
endif (HAS_MISSING_PROTOTYPES)

if (HAS_MISSING_DECLARATIONS)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations")
endif (HAS_MISSING_DECLARATIONS)

if (HAS_SIGN_COMPARE)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wsign-compare")
endif (HAS_SIGN_COMPARE)

CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
if (NOT HAVE_INTTYPES_H)
  MESSAGE(FATAL_ERROR "Missing inttypes.h")
endif (NOT HAVE_INTTYPES_H)

CHECK_INCLUDE_FILES (stdio.h HAVE_STDIO_H)
if (NOT HAVE_STDIO_H)
  MESSAGE(FATAL_ERROR "Missing stdio.h")
endif (NOT HAVE_STDIO_H)

CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB_H)
if (NOT HAVE_STDLIB_H)
  MESSAGE(FATAL_ERROR "Missing stdlib.h")
endif (NOT HAVE_STDLIB_H)

CHECK_INCLUDE_FILES (string.h HAVE_STRING_H)
if (NOT HAVE_STRING_H)
  MESSAGE(FATAL_ERROR "Missing string.h")
endif (NOT HAVE_STRING_H)

CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)
if (NOT HAVE_UNISTD_H)
  MESSAGE(FATAL_ERROR "Missing unistd.h")
endif (NOT HAVE_UNISTD_H)

CHECK_INCLUDE_FILES (sys/dir.h HAVE_DIR_H)
if (NOT HAVE_DIR_H)
  MESSAGE(FATAL_ERROR "Missing sys/dir.h")
endif (NOT HAVE_DIR_H)

CHECK_INCLUDE_FILES (sys/stat.h HAVE_STAT_H)
if (NOT HAVE_STAT_H)
  MESSAGE(FATAL_ERROR "Missing sys/stat.h")
endif (NOT HAVE_STAT_H)

CHECK_INCLUDE_FILES (sys/param.h HAVE_STAT_H)
if (NOT HAVE_STAT_H)
  MESSAGE(FATAL_ERROR "Missing sys/param.h")
endif (NOT HAVE_STAT_H)

CHECK_INCLUDE_FILES (fcntl.h HAVE_FCNTL_H)
if (NOT HAVE_FCNTL_H)
  MESSAGE(FATAL_ERROR "Missing fcntl.h")
endif (NOT HAVE_FCNTL_H)

CHECK_INCLUDE_FILES (time.h HAVE_TIME_H)
if (NOT HAVE_TIME_H)
  MESSAGE(FATAL_ERROR "Missing time.h")
endif (NOT HAVE_TIME_H)

CHECK_INCLUDE_FILES (ctype.h HAVE_CTYPE_H)
if (NOT HAVE_CTYPE_H)
  MESSAGE(FATAL_ERROR "Missing ctype.h")
endif (NOT HAVE_CTYPE_H)

CHECK_INCLUDE_FILES (math.h HAVE_MATH_H)
CHECK_LIBRARY_EXISTS(m pow "" HAVE_M_LIB)
if (NOT HAVE_MATH_H OR NOT HAVE_M_LIB)
  MESSAGE(FATAL_ERROR "Math library is required")
endif (NOT HAVE_MATH_H OR NOT HAVE_M_LIB)

CHECK_INCLUDE_FILES (dlfcn.h HAVE_DLFCN_H)
CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_DLFCN_LIB)
if (NOT HAVE_DLFCN_H OR NOT HAVE_DLFCN_LIB)
  MESSAGE(FATAL_ERROR "No dlopen() found in your system")
endif (NOT HAVE_DLFCN_H OR NOT HAVE_DLFCN_LIB)

CHECK_INCLUDE_FILES (popt.h HAVE_POPT_H)
CHECK_LIBRARY_EXISTS(popt poptGetContext "" HAVE_POPT_LIB)
if (NOT HAVE_POPT_H OR NOT HAVE_POPT_LIB)
  MESSAGE(FATAL_ERROR "Popt library is required")
endif (NOT HAVE_POPT_H OR NOT HAVE_POPT_LIB)

CHECK_INCLUDE_FILES (mpi.h HAVE_MPI_H)
CHECK_LIBRARY_EXISTS(mpi MPI_Bcast "" HAVE_MPI_LIB)
if (NOT HAVE_MPI_H OR NOT HAVE_MPI_LIB)
  MESSAGE(FATAL_ERROR "MPI2 not found, please check the MPI installation")
endif (NOT HAVE_MPI_H OR NOT HAVE_MPI_LIB)

CHECK_INCLUDE_FILES (hdf5.h HAVE_HDF5_H)
CHECK_LIBRARY_EXISTS(hdf5 H5Dopen2 "" HAVE_HDF5_LIB)
if (NOT HAVE_HDF5_H OR NOT HAVE_HDF5_LIB)
  MESSAGE(FATAL_ERROR "HDF5 not found, please check the HDF5 installation")
endif (NOT HAVE_HDF5_H OR NOT HAVE_HDF5_LIB)

CONFIGURE_FILE (
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mechanic_config.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/src/mechanic_config.h
)

add_subdirectory(src)
add_subdirectory(src/core)
add_subdirectory(src/modes)
add_subdirectory(src/modules)
add_subdirectory(examples)
add_subdirectory(tests)

if (BUILD_PYTHON_TOOLKIT)
  add_subdirectory(src/toolkits/python)
endif (BUILD_PYTHON_TOOLKIT)

if (BUILD_VENDOR_RNGS)
  add_subdirectory(src/vendor/rngs)
endif (BUILD_VENDOR_RNGS)

if (BUILD_MODULES)
    add_subdirectory(examples/modules)
endif (BUILD_MODULES)

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
  "/build/;/.DS_Store;/.gitignore;/.git/;~$;*.so;*.dylib;*.h5;${CPACK_SOURCE_IGNORE_FILES}")

include (CPack)
include (CTest)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

