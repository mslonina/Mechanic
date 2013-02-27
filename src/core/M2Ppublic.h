/**
 * @file
 * Public task pool interface
 */
#ifndef MECHANIC_M2P_PUBLIC_H
#define MECHANIC_M2P_PUBLIC_H

#include "M2Spublic.h"
#include "M2Tpublic.h"

/* Pool */
#define POOL_FINALIZE 1001 /**< The pool finalize return code */
#define POOL_RESET 1002 /**< The pool reset return code */
#define POOL_CREATE_NEW 1003 /**< The pool create new return code */
#define POOL_STAGE 1004 /**< The pool stage return code */
#define POOL_STAGE_RESET 1005 /**< The pool stage return code */
#define POOL_PREPARED 2001 /**< Pool prepared state */
#define POOL_PROCESSED 2002 /**< Pool processed state */

#endif

