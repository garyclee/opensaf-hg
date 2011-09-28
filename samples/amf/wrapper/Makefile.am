#      -*- OpenSAF  -*-
#
# (C) Copyright 2011 The OpenSAF Foundation
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
# Author(s): Ericsson
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	wrapper.sh \
	AppConfig-2N.xml \
	AppConfig-nwayactive.xml \
	README

sbin_PROGRAMS = wrapper

wrapper_CPPFLAGS = \
	$(AM_CPPFLAGS)

wrapper_SOURCES = \
	wrapper.c

wrapper_LDADD = \
	@SAF_AIS_AMF_LIBS@

install-data-hook:
	cp wrapper.sh /usr/local/sbin

uninstall-hook:
	rm -f /usr/local/sbin/wrapper.sh

