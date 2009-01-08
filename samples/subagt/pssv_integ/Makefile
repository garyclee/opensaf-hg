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

LIB = libsnmptm_pssv.so

PSSV_GENERATED_SRCS = \
	ncstestscalars_pssv_mib.c \
	ncstesttableeightentry_pssv_mib.c \
	ncstesttablefourentry_pssv_mib.c \
	ncstesttablenineentry_pssv_mib.c \
	ncstesttableoneentry_pssv_mib.c \
	ncstesttablesevenentry_pssv_mib.c \
	ncstesttablesixentry_pssv_mib.c \
	ncstesttabletenentry_pssv_mib.c \
	ncstesttablethreeentry_pssv_mib.c \
	snmptm_pssv_init.c

OBJS = $(PSSV_GENERATED_SRCS:.c=.o)

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

$(PSSV_GENERATED_SRCS): pssv

pssv:
	@(export SMIPATH=/usr/share/snmp/mibs:/usr/share/opensaf/mibs:../mibs; \
		smidump -R ../OpenSAFCopyRights \
		-f ncspssv \
		-d ../ \
		-o snmptm NCS-APPLTEST-MIB;)

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	rm -f *.[cho] core *~ $(LIB)
