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
 * Author(s): Ericsson AB
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "logtrace.h"

static int trace_fd = -1;
static int category_mask;
static char *prefix_name[] = {"EM", "AL", "CR", "ER", "WA", "NO", "IN", "DB",
    "TR", "T1", "T2", "T3", "T4", "T5", "T6", "T7", "T8", ">>", "<<"};
static const char *my_name;

static void output(const char* file, unsigned int line, int priority, int category,
                  const char *format, va_list ap)
{
    int i, preamble_len;
    struct timeval tv;
    char preamble[512];
    char log_string[1024];

    assert(priority <= LOG_DEBUG && category < CAT_MAX);

    /* Create a nice syslog looking date string */
    gettimeofday(&tv, NULL);
    strftime(log_string, sizeof(log_string), "%b %e %k:%M:%S", localtime(&tv.tv_sec));
    preamble_len = sprintf(preamble, "%s.%06ld %s ", log_string, tv.tv_usec, my_name);

    sprintf(&preamble[preamble_len], "[%ld:%s:%04u] %s %s",
            syscall(SYS_gettid), file, line, prefix_name[priority + category], format);
    i = vsprintf(log_string, preamble, ap);

    /* Add line feed if not there already */
    if (log_string[i - 1] != '\n')
    {
        log_string[i] = '\n';
        log_string[i + 1] = '\0';
        i++;
    }

    if (trace_fd != -1)
        write(trace_fd, log_string, i);
}

void _logtrace_log(const char* file, unsigned int line, int priority,
    const char *format, ...)
{
    va_list ap;

    va_start(ap, format);

    /* Uncondionally send to syslog */
    vsyslog(priority, format, ap);

    /* Only output to file if configured to */
    if (!(category_mask & (1 << CAT_LOG)))
        return;

    output(file, line, priority, CAT_LOG, format, ap);
    va_end(ap);
}

void _logtrace_trace(const char* file, unsigned int line,
    unsigned int category, const char *format, ...)
{
    va_list ap;

    /* Filter on category */
    if (!(category_mask & (1 << category)))
        return;

    va_start(ap, format);
    output(file, line, LOG_DEBUG, category, format, ap);
    va_end(ap);
}

int logtrace_init(const char *ident, const char *pathname)
{
    if (ident == NULL || pathname == NULL)
        return -1;

    my_name = ident;
    openlog(ident, 0, LOG_LOCAL0);

    trace_fd = open(pathname, O_WRONLY | O_APPEND | O_CREAT,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (trace_fd == -1)
        return -1;
    else
        return 0;
}

int trace_category_set(unsigned int category)
{
    /* Do not enable tracing without an output device */
    if (trace_fd == -1)
        return -1;

    category_mask = category;

    return 0;
}

