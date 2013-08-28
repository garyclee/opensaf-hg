#      -*- OpenSAF  -*-
#
# (C) Copyright 2009 The OpenSAF Foundation
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

SUBDIRS = amf immsv logsv ntfsv clmsv

if ENABLE_AIS_PLM

SUBDIRS += plmsv

endif

if ENABLE_AIS_CKPT

SUBDIRS += cpsv

endif

if ENABLE_AIS_EVT

SUBDIRS += edsv

endif

if ENABLE_AIS_LCK

SUBDIRS += glsv

endif

if ENABLE_AIS_MSG

SUBDIRS += mqsv

endif

if ENABLE_AIS_SMF

SUBDIRS += smfsv

endif
