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

PROG = snmptm_demo

MIBLIB_GENERATED_SRCS = \
	snmptm_miblibgen/ncstestscalars_mib.c \
	snmptm_miblibgen/ncstesttableeightentry_mib.c \
	snmptm_miblibgen/ncstesttablefiveentry_mib.c \
	snmptm_miblibgen/ncstesttablefourentry_mib.c \
	snmptm_miblibgen/ncstesttablenineentry_mib.c \
	snmptm_miblibgen/ncstesttableoneentry_mib.c \
	snmptm_miblibgen/ncstesttablesevenentry_mib.c \
	snmptm_miblibgen/ncstesttablesixentry_mib.c \
	snmptm_miblibgen/ncstesttabletenentry_mib.c \
	snmptm_miblibgen/ncstesttablethreeentry_mib.c

SRCS = snmptm_main.c \
	snmptm_api.c \
	snmptm_bgn.c \
	snmptm_cb.c \
	snmptm_mib.c \
	snmptm_scalar_mib.c \
	snmptm_tblcmd.c \
	snmptm_tbleight.c \
	snmptm_tblfive.c \
	snmptm_tblfive_mib.c \
	snmptm_tblfour.c \
	snmptm_tblone.c \
	snmptm_tblone_mib.c \
	snmptm_tblseven.c \
	snmptm_tblsix.c \
	snmptm_tblsix_mib.c \
	snmptm_tblten.c \
	snmptm_tblthree.c \
	snmptm_tblthree_mib.c

OBJS = $(SRCS:.c=.o) $(MIBLIB_GENERATED_SRCS:.c=.o)

CC = gcc
CFLAGS = -g -O2 -Wall -fPIC
INCLUDES = -I. -I./inc -I/usr/include/opensaf
CPPFLAGS = -DNCS_SAF=1
LDFLAGS = -lncs_core -lsaf_common -lSaEvt -ledsv_common

all: $(PROG)
	$(MAKE) -C pssv_integ
	$(MAKE) -C snmp_integ

$(PROG): $(OBJS)
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c $<

snmptm_miblibgen/%.o: snmptm_miblibgen/%.c
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDES} -c -o $@ $<

$(MIBLIB_GENERATED_SRCS): miblib

miblib:
	@(export SMIPATH=/usr/share/snmp/mibs:/usr/share/opensaf/mibs:./mibs; \
		smidump -R ./OpenSAFCopyRights \
		-f ncsmiblib \
		-d ./ \
		NCS-APPLTEST-MIB;)

depend:
	makedepend ${INCLUDES} ${SRCS}

clean:
	$(MAKE) -C pssv_integ clean
	$(MAKE) -C snmp_integ clean
	rm -f *.o core *~ $(PROG)
	rm -rf TEMP_ESMI_DUMP.MIB snmptm_miblibgen/
