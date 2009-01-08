/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Nokia Siemens Networks, OptXware Research & Development LLC.
 */

#ifndef J_AIS_H_
#define J_AIS_H_

/**************************************************************************
 * Include files
 *************************************************************************/

#include <jni.h>

/**************************************************************************
 * Constants
 *************************************************************************/

extern const char* AIS_ERR_LIBRARY_MSG;
extern const char* AIS_ERR_VERSION_MSG;
extern const char* AIS_ERR_INIT_MSG;
extern const char* AIS_ERR_TIMEOUT_MSG;
extern const char* AIS_ERR_TRY_AGAIN_MSG;
extern const char* AIS_ERR_INVALID_PARAM_MSG;
extern const char* AIS_ERR_NO_MEMORY_MSG;
extern const char* AIS_ERR_BAD_HANDLE_MSG;
extern const char* AIS_ERR_BUSY_MSG;
extern const char* AIS_ERR_ACCESS_MSG;
extern const char* AIS_ERR_NOT_EXIST_MSG;
extern const char* AIS_ERR_NAME_TOO_LONG_MSG;
extern const char* AIS_ERR_EXIST_MSG;
extern const char* AIS_ERR_NO_SPACE_MSG;
extern const char* AIS_ERR_INTERRUPT_MSG;
extern const char* AIS_ERR_NAME_NOT_FOUND_MSG;
extern const char* AIS_ERR_NOT_SUPPORTED_MSG;
extern const char* AIS_ERR_BAD_OPERATION_MSG;
extern const char* AIS_ERR_FAILED_OPERATION_MSG;
extern const char* AIS_ERR_NO_RESOURCES_MSG;
extern const char* AIS_ERR_MESSAGE_ERROR_MSG;
extern const char* AIS_ERR_QUEUE_FULL_MSG;
extern const char* AIS_ERR_QUEUE_NOT_AVAILABLE_MSG;
extern const char* AIS_ERR_BAD_FLAGS_MSG;
extern const char* AIS_ERR_TOO_BIG_MSG;
extern const char* AIS_ERR_NO_SECTIONS_MSG;

/**************************************************************************
 * Macros
 *************************************************************************/

/**************************************************************************
 * Data types and structures
 *************************************************************************/

/**************************************************************************
 * Variable declarations
 *************************************************************************/

// CLASS ais.SVersion
extern jfieldID FID_releaseCode;
extern jfieldID FID_majorVersion;
extern jfieldID FID_minorVersion;

// ENUM ais.TrackFlags
extern jfieldID FID_TF_value;


/**************************************************************************
 * Function declarations
 *************************************************************************/

// CLASS ais.SVersion
extern jboolean JNU_Version_initIDs_OK(
    JNIEnv* jniEnv );

// ENUM ais.TrackFlags
extern jboolean JNU_TrackFlags_initIDs_OK(
    JNIEnv* jniEnv );
// MISC
/*
extern jboolean JNU_TrackFlagsForChanges_OK(
    JNIEnv* jniEnv,
    jbyte trackFlags );
*/


#endif //J_AIS_H_
