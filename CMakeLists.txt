cmake_minimum_required (VERSION 2.8)
project (mechanic)

set (Mechanic_VERSION_MAJOR 0)
set (Mechanic_VERSION_MINOR 12)
set (Mechanic_VERSION_PATCH 0-Unstable-3.2)

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

option (BUILD_FORTRAN "Build Fortran bindings" off)
option (BUILD_DOCS "Build documentation" off)

if (BUILD_FORTRAN)
  enable_language (Fortran)
  set (CMAKE_Fortran_FLAGS "-std=f2003 -fbounds-check -fbacktrace -ffpe-trap=zero,overflow -fdefault-real-8")
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

CONFIGURE_FILE (
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/src/config.h
)

add_definitions (-Wall -std=c99 -DHAVE_CONFIG_H)

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
SET (CPACK_SOURCE_IGNORE_FILES
  "/build/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")

INCLUDE(CPACK)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

