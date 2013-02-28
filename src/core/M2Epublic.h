/**
 * @file
 * Error and message handlers (public API)
 */
#ifndef MECHANIC_M2E_PUBLIC_H
#define MECHANIC_M2E_PUBLIC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <mpi.h>
#include <hdf5.h>

#define ICE_FILENAME "mechanic.ice" /**< The Mechanic ICE file */

#define SUCCESS 0 /**< The success return code */
#define CORE_ICE 112 /**< The core emergency return code */
#define CORE_SETUP_HELP 212 /**< The core help message return code */
#define CORE_SETUP_USAGE 213 /**< The core usage message return code */

/* Error codes */
#define CORE_ERR_CORE 901 /**< The core related error */
#define CORE_ERR_MPI 911 /**< The core MPI related error */
#define CORE_ERR_HDF 912 /**< The core HDF related error */
#define CORE_ERR_MODULE 913 /**< The core module related error */
#define CORE_ERR_SETUP 914 /**< The core setup related error */
#define CORE_ERR_MEM 915 /**< The core memory related error */
#define CORE_ERR_CHECKPOINT 916 /**< The core checkpoint related error */
#define CORE_ERR_STORAGE 917 /**< The core storage related error */
#define CORE_ERR_RESTART 918 /**< The core restart mode related error */
#define CORE_ERR_OTHER 999 /**< The core any other error */

#define MODULE_ERR_CORE 801 /**< The module related error */
#define MODULE_ERR_MPI 811 /**< The module MPI related error */
#define MODULE_ERR_HDF 812 /**< The module HDF related error */
#define MODULE_ERR_SETUP 814 /**< The module setup related error */
#define MODULE_ERR_MEM 815 /**< The module memory related error */
#define MODULE_ERR_CHECKPOINT 816 /**< The module checkpoint related error */
#define MODULE_ERR_OTHER 888 /**< The module any other error */

/**
 * @enum MessageType
 * The types of messages
 */
typedef enum {
  MESSAGE_INFO, /**< The message info type */
  MESSAGE_ERR, /**< The message error type (only the message) */
  MESSAGE_WARN, /**< The warning message type */
  MESSAGE_DEBUG, /**< The debug message type */
  MESSAGE_OUTPUT, /**< The clear output message type */
  MESSAGE_RESULT, /**< The result output message type */
  MESSAGE_COMMENT, /**< The comment message type */
} MessageType;

/**
 * Error and message helpers
 */
int Ice(); /**< Checks for the ICE file */
void Message(int type, char* message, ...); /**< Common printf wrapper */
void Error(int status); /**< Error reporting */
void Abort(int status); /**< Abort handler */
void CheckStatus(int status); /**< Status checking utility*/
void H5CheckStatus(hid_t status); /**< HDF5 status checking utility*/

#endif

