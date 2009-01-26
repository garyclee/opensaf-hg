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

PROG1 = maa_demo
PROG2 = trap_demo

SRCS1 = maa_api_usage.cpp
OBJS1 = maa_api_usage.o

SRCS2 = ncs_trap_usage.cpp
OBJS2 = ncs_trap_usage.o

CXX = g++
CXXFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I/usr/include/opensaf -I../subagt/inc
CPPFLAGS = -DNCS_SAF=1 -DNCS_DEBUG=1
LDFLAGS = -lncs_core -lmaa -lSaEvt

all: $(PROG1) $(PROG2)

$(PROG1): $(OBJS1)
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${INCLUDES} -o $@ ${OBJS1} ${LDFLAGS}

$(PROG2): $(OBJS2)
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${INCLUDES} -o $@ ${OBJS2} ${LDFLAGS}

.cpp.o:
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${INCLUDES} -c $<

depend:
	makedepend ${INCLUDES} ${SRCS1}
	makedepend ${INCLUDES} ${SRCS2}

clean:
	rm -f *.o core *~ $(PROG1) $(PROG2)
