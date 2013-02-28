/**
 * @file
 * Error and message handlers (public API)
 */
#include "M2Epublic.h"

/**
 * @brief Common error handler
 * 
 * We abort on any error code
 * @param errcode The error code to use
 */
void Error(int errcode) {
  Abort(errcode);
}

/**
 * @brief Wrapper to MPI_Abort
 *
 * @param errcode The error code to use
 */
void Abort(int errcode) {
  MPI_Abort(MPI_COMM_WORLD, errcode);
}

/**
 * @brief Common status check
 *
 * Throw an exception on any error code
 *
 * @param status The status code to check for
 */
void CheckStatus(int status) {
  if (status >= MODULE_ERR_CORE && status <= CORE_ERR_OTHER) Error(status);
}

/**
 * @brief HDF5 status check
 *
 * @param status The status code to check for
 */
void H5CheckStatus(hid_t status) {
  if (status < 0) Error(CORE_ERR_HDF);
}

/**
 * @brief Common messaging interface
 *
 * @param type The type of the message
 * @param message The message to display
 */
void Message(int type, char *message, ...) {
  static char message2[2048];
  va_list args;

  va_start(args, message);
    vsprintf(message2, message, args);
    if (type == MESSAGE_INFO)    printf("-- %s", message2);
    if (type == MESSAGE_COMMENT) printf("#  %s", message2);
    if (type == MESSAGE_OUTPUT)  printf("   %s", message2);
    if (type == MESSAGE_RESULT)  printf(" > %s", message2);
    if (type == MESSAGE_ERR)     printf("!! %s", message2);
		if (type == MESSAGE_WARN)    printf(".. %s", message2);
#ifdef WITH_DEBUG
    if (type == MESSAGE_DEBUG)   printf("%s", message2);
#endif
  va_end(args);
}

/**
 * @brief Check for the ICE file
 *
 * @return CORE_ICE if the file has been found, SUCCESS otherwise
 */
int Ice() {
  struct stat file;
  if (stat(ICE_FILENAME, &file) == 0) return CORE_ICE;
  return SUCCESS;
}

