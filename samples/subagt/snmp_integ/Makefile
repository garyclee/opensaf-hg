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

LIB = libsnmptm_subagt.so

SUBAGT_GENERATED_SRCS = \
	snmptm_subagt_init.c \
	mcg_ncs_test_mib_agt_stub.c

OBJS = $(SUBAGT_GENERATED_SRCS:.c=.o)

CC = gcc
CFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I/usr/include/opensaf -I../inc
CPPFLAGS = -DNCS_SAF=1
LDFLAGS = -lncs_core -lsaf_common -lSaEvt -ledsv_common

all: $(LIB)

$(LIB): $(OBJS)
	${CC} -shared -Wl,-soname,$(LIB) -o $(LIB) ${OBJS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c $<

$(SUBAGT_GENERATED_SRCS): subagt

subagt:
	@(export SMIPATH=/usr/share/snmp/mibs:/usr/share/opensaf/mibs:../mibs; \
		smidump -R ../OpenSAFCopyRights \
		-f synncsnetsnmp_new \
		-d ../ \
		-o snmptm NCS-APPLTEST-MIB;)

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	rm -f *.[cho] core *~ $(LIB)
