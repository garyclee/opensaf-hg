/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2012 The OpenSAF Foundation
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
 * Author(s): Ericsson AB
 *
 */

/** @file osaf_utility.h
 *
 * This file contains utility/convenience functions provided by libopensaf_core.
 * The definitions in this file are for internal use within OpenSAF only.
 */

#ifndef OPENSAF_CORE_OSAF_UTILITY_H_
#define OPENSAF_CORE_OSAF_UTILITY_H_

#include <stdint.h>
#include <pthread.h>

#ifdef	__cplusplus
extern "C" {
#endif

static inline void osaf_mutex_lock_ordie(pthread_mutex_t* __restrict i_mutex,
	const char* __restrict i_file, uint32_t i_line)
	__attribute__ ((nonnull, nothrow));
static inline void osaf_mutex_unlock_ordie(pthread_mutex_t* __restrict i_mutex,
	const char* __restrict i_file, uint32_t i_line)
	__attribute__ ((nonnull, nothrow));

/**
 * @brief Log an error message and abort the process.
 * @param[in] i_file File name of caller (use the __FILE__ macro)
 * @param[in] i_line Line number of caller (use the __LINE__ macro)
 * @param[in] i_info1 Extra information word 1
 * @param[in] i_info2 Extra information word 2
 * @param[in] i_info3 Extra information word 3
 * @param[in] i_info4 Extra information word 4
 *
 * This is a convenience function that calls syslog(3) to log an error message,
 * and then aborts the process by calling abort(3). The error message will
 * contain information about the file name and line number where the error
 * condition was detected, provided by the caller in the i_file and i_line
 * parameters. The caller can optionally also provide up to four additional
 * numerical data in the i_info1 through i_info4 parameters. In addition, this
 * function will log the current value of errno, so the caller does not have
 * to include the errno value in the extra information words unless errno may
 * have changed since the point of error.
 */
extern void osaf_abort(const char* __restrict i_file, uint32_t i_line,
	uint32_t i_info1, uint32_t i_info2, uint32_t i_info3, uint32_t i_info4)
	__attribute__ ((nonnull, nothrow, noreturn));

/**
 * @brief Lock a pthreads mutex and abort the process in case of failure.
 * @param[in] i_mutex Pointer to a pthreads mutex to be locked.
 * @param[in] i_file File name of caller (use the __FILE__ macro)
 * @param[in] i_line Line number of caller (use the __LINE__ macro)
 *
 * This is a convenience function that behaves exactly like the POSIX function
 * pthread_mutex_lock(3P), except that instead of returning an error code it
 * aborts the process in case of an error. The caller shall provide file name
 * and line number for point of call, and this information is logged to syslog
 * before aborting the process.
 */
static inline void osaf_mutex_lock_ordie(pthread_mutex_t* __restrict i_mutex,
	const char* __restrict i_file, uint32_t i_line)
{
	int result = pthread_mutex_lock(i_mutex);
	if (result != 0) osaf_abort(i_file, i_line, result, 0, 0, 0);
}

/**
 * @brief Unlock a pthreads mutex and abort the process in case of failure.
 * @param[in] i_mutex Pointer to a pthreads mutex to be unlocked.
 * @param[in] i_file File name of caller (use the __FILE__ macro)
 * @param[in] i_line Line number of caller (use the __LINE__ macro)
 *
 * This is a convenience function that behaves exactly like the POSIX function
 * pthread_mutex_unlock(3P), except that instead of returning an error code it
 * aborts the process in case of an error. The caller shall provide file name
 * and line number for point of call, and this information is logged to syslog
 * before aborting the process.
 */
static inline void osaf_mutex_unlock_ordie(pthread_mutex_t* __restrict i_mutex,
	const char* __restrict i_file, uint32_t i_line)
{
	int result = pthread_mutex_unlock(i_mutex);
	if (result != 0) osaf_abort(i_file, i_line, result, 0, 0, 0);
}

#ifdef	__cplusplus
}
#endif

#endif	/* OPENSAF_CORE_OSAF_UTILITY_H_ */
