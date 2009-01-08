#      -*- OpenSAF  -*-
#
# (C) Copyright 2008 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Wind River Systems
#

PROG = glsv_demo

SRCS = glsv_demo_app1.c glsv_main_app1.c glsv_test_app.c
OBJS = glsv_demo_app1.o glsv_main_app1.o glsv_test_app.o

CC = gcc
CFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I/usr/include/opensaf
CPPFLAGS = -DNCS_SAF=1
LDFLAGS = -lncs_core -lSaLck

all: $(PROG)

$(PROG): $(OBJS)
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c $<

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	rm -f *.o core *~ $(PROG)
