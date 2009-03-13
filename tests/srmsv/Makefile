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
# tests/srmsv/Makefile
#

include ../Makefile.common

EXE := reg_srmsv.exe

SRCS := $(wildcard ./src/*.c)
OBJECTS := $(patsubst %.c,%.o,$(SRCS))

CPPFLAGS += -DNCS_SRMA=1 -DTET_SRMSV=1 -DTET_A=1

LDLIBS += -lsrma
ifeq "$(OS)" "SunOS"
LDLIBS += -lsocket -lnsl
endif


all:  $(OBJECTS) $(EXE)
	mv $(EXE) suites

clobber clean:
	$(RM) $(OBJECTS) suites/$(EXE)

%.o: %.c
	$(COMPILER) $(CPPFLAGS) $(CFLAGS) -o $@ $<


$(EXE): $(OBJECTS)
	$(COMPILER) $(OBJECTS) -L$(LIBDIR) $(LDLIBS) -o $@
