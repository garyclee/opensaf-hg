#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = comp_local

EXTRA_DIST = \
	amf_demo_script \
	AppConfig-2N.xml \
	AppConfig-nwayactive.xml \
	README

bin_PROGRAMS = amf_demo

amf_demo_CPPFLAGS = \
	$(AM_CPPFLAGS)

amf_demo_SOURCES = \
	amf_comp_template.c

amf_demo_LDADD = \
	@SAF_AIS_AMF_LIBS@

install-data-hook:
	mkdir -p /opt/amf_demo
	cp -r amf_demo_script /opt/amf_demo

uninstall-hook:
	rm -rf /opt/amf_demo

