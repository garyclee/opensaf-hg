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

PROG = hisv_events_demo

SRCS = hisv_events_demo_app.c hisv_events_demo_main.c
OBJS = hisv_events_demo_app.o hisv_events_demo_main.o

CC = gcc
CFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I/usr/include/opensaf -I/usr/include/openhpi
CPPFLAGS = -DNCS_SAF=1
LDFLAGS = -lSaAmf -ledsv_common -lSaEvt -lhpl

all: $(PROG)

$(PROG): $(OBJS)
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c $<

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	rm -f *.o core *~ $(PROG)
