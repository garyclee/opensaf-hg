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

#
# tests/ifsv/Makefile
#

include ../Makefile.common

EXE := reg_ifsv.exe

DRIVER := reg_ifsv_driver.exe

VIP := reg_ifsv_vip.exe

OBJECTS := \
	src/tet_ifa.o \
	src/tet_ifsv_driver.o \
	src/tet_ifsv_ipxs.o \
	src/tet_ifsv_wrappers.o\
	src/tet_ifsv_util.o

#
# Build DRIVER objects with "drv.o" suffix to distinguish them
# from OBJECTS, which are compiled using different CPPFLAGS.
#
DRIVEROBJECTS := \
	src/tet_ifa.drv.o \
	src/tet_ifsv_driver.drv.o \
	src/tet_ifsv_ipxs.drv.o \
	src/tet_ifsv_wrappers.drv.o\
	src/tet_ifsv_util.drv.o

VIPOBJECTS := \
	vip/vip_ifa.o \
        vip/tet_ifsv_vip_util.o

CPPFLAGS += -DTET_IFSV=1
ECPPFLAGS := -DEEEEEEEEE $(CPPFLAGS) -DNCS_IFA=1 -DNCS_IFSV_IPXS=1 -DTET_A=1 -I../edsv/inc
DCPPFLAGS := -DDDDDDDDDD $(CPPFLAGS) -DNCS_IFND=1 -DNCS_IFSV_LOG=1 -DNCS_IFSV_IPXS=1 -DTET_DRIVER=1 -I../edsv/inc
VCPPFLAGS := -DVVVVVVVVV $(CPPFLAGS) -DNCS_IFA=1 -DNCS_IFSV_IPXS=1 -DTET_A=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1

ELDLIBS := $(LDLIBS) -lifsv_common -ledsv_common -lsaf_common -lifa -lSaEvt -lmaa
ifeq "$(OS)" "SunOS"
ELDLIBS += -lsocket -lnsl
endif
DLDLIBS := $(LDLIBS) -lifsv_common -ledsv_common -lsaf_common -lifa -lSaEvt -lmaa
ifeq "$(OS)" "SunOS"
DLDLIBS += -lsocket -lnsl
endif
VLDLIBS := $(LDLIBS) -lifsv_common -ledsv_common -lsaf_common -lifa
ifeq "$(OS)" "SunOS"
VLDLIBS += -lsocket -lnsl
endif

TET_INIT_OBJ := tet_init.o

all: $(TET_INIT_OBJ) $(OBJECTS) $(DRIVEROBJECTS) $(VIPOBJECTS) \
	$(EXE) $(DRIVER) \
	$(VIP)
	mv $(EXE) suites
	mv $(DRIVER) suites
	mv $(VIP) suites

clobber clean:
	$(RM) $(TET_INIT_OBJ) $(OBJECTS) $(DRIVEROBJECTS) $(VIPOBJECTS) \
	suites/$(EXE) suites/$(DRIVER) \
	suites/$(VIP)

$(OBJECTS): %.o: %.c
	$(COMPILER) $(ECPPFLAGS) $(CFLAGS) -o $@ $<

$(DRIVEROBJECTS): %.drv.o: %.c
	$(COMPILER) $(DCPPFLAGS) $(CFLAGS) -o $@ $<

$(VIPOBJECTS): %.o: %.c
	$(COMPILER) $(VCPPFLAGS) $(CFLAGS) -o $@ $<

$(TET_INIT_OBJ): ../common/src/tet_init.c
	$(COMPILER) $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(EXE): $(TET_INIT_OBJ) $(OBJECTS)
	$(COMPILER) $(TET_INIT_OBJ) $(OBJECTS) -L$(LIBDIR) $(ELDLIBS) -o $@

$(DRIVER): $(TET_INIT_OBJ) $(DRIVERSOURCES) $(DRIVEROBJECTS)
	$(COMPILER) $(TET_INIT_OBJ) $(DRIVEROBJECTS) -L$(LIBDIR) $(DLDLIBS) -o $@

$(VIP): $(TET_INIT_OBJ) $(VIPOBJECTS)
	$(COMPILER) $(TET_INIT_OBJ) $(VIPOBJECTS) -L$(LIBDIR) $(VLDLIBS) -o $@
