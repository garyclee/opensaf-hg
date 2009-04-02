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

PROG = dtsv_demo
LIB = libdummy_logstr.so

SRCS = dtsv_demo_main.c dtsv_dummy_ss1.c
OBJS = dtsv_demo_main.o dtsv_dummy_ss1.o

CC = gcc
CFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I/usr/include/opensaf
CPPFLAGS = -DNCS_SAF=1 -DNCS_DTS=1
LDFLAGS = -lncs_core

all: $(PROG) $(LIB)

$(LIB): dtsv_dummy1_str.o
	${CC} -shared -Wl,-soname,$(LIB) -o $(LIB) dtsv_dummy1_str.o

dtsv_dummy1_str.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c dtsv_dummy1_str.c

$(PROG): $(OBJS)
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c $<

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	rm -f *.o core *~ $(PROG) $(LIB)
