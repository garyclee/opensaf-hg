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

PROG = leaptest_demo
LIB = liblttest_dl_app.so

SRCS = test_atomic_counts.c test_bufmgr.c test_encdec.c test_file.c \
       test_locks.c test_mailbox.c test_main.c test_memdiag.c \
		 test_patricia.c test_processlib.c test_timers.c

OBJS = test_atomic_counts.o test_bufmgr.o test_encdec.o test_file.o \
       test_locks.o test_mailbox.o test_main.o test_memdiag.o \
		 test_patricia.o test_processlib.o test_timers.o

CC = gcc
CFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I/usr/include/opensaf
CPPFLAGS = -DNCS_SAF=1 -DNCS_IFSV_BOND_INTF=1
LDFLAGS = -lncs_core

all: $(PROG) $(LIB)

$(PROG): $(OBJS)
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}

$(LIB): test_dl_app.o
	${CC} -shared -Wl,-soname,$(LIB) -o $(LIB) test_dl_app.o

test_dl_app.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c test_dl_app.c

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c $<

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	rm -f *.o core *~ $(PROG) $(LIB)
