#      -*- OpenSAF  -*-
#
# (C) Copyright 2013 The OpenSAF Foundation
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
	proxy_script \
	proxied_script \
	proxy.xml \
	proxied.xml \
	README

bin_PROGRAMS = proxy

proxy_CPPFLAGS = \
	$(AM_CPPFLAGS)

proxy_SOURCES = \
	proxy.c

proxy_LDADD = \
	@SAF_AIS_AMF_LIBS@

install-data-hook:
	mkdir -p /opt/amf_proxy_demo
	cp proxy proxy_script proxied_script /opt/amf_proxy_demo

uninstall-hook:
	rm -rf /opt/amf_proxy_demo

