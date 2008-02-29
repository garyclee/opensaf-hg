#      -*- OpenSAF  -*-
#
#  Copyright (c) 2007, Ericsson AB
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under High-Availability Operating 
# Environment Software License Version 1.4.
# Complete License can be accesseble from below location.
# http://www.opensaf.org/license 
# See the Copying file included with the OpenSAF distribution for
# full licensing terms.
#
# Author(s):
#   
#

CFLAGS := -Wall -I$(OPENSAF_INCLUDE) -I.
LDFLAGS := -L$(OPENSAF_LIB) -lSaAmf

aishello: aishello.c ais_utils.c
	gcc -o $@ $(CFLAGS) $^ $(LDFLAGS)
	strip $@

clean:
	rm -f aishello *.o
