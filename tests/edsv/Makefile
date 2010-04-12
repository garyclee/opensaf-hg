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
# tests/edsv/Makefile
#

include ../Makefile.common

EXE := reg_edsv.exe

SRCS := $(wildcard ./src/*.c)
OBJECTS := $(patsubst %.c,%.o,$(SRCS))

CPPFLAGS += -DTET_EDSV=1

LDLIBS += -lSaEvt
ifeq "$(OS)" "SunOS"
LDLIBS += -lsocket -lnsl
endif

TET_INIT_OBJ := tet_init.o

all: $(TET_INIT_OBJ) $(OBJECTS) $(EXE)
	mv $(EXE) suites
	
clobber clean:
	$(RM) $(TET_INIT_OBJ) $(OBJECTS) suites/$(EXE)

%.o: %.c
	$(COMPILER) $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(TET_INIT_OBJ): ../common/src/tet_init.c
	$(COMPILER) $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(EXE): $(OBJECTS)
	$(COMPILER) $(TET_INIT_OBJ) $(OBJECTS) -L$(LIBDIR) $(LDLIBS) -o $@
