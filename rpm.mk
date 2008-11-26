#      -*- OpenSAF  -*-
#      #
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

RPM_WITH_OPTIONS_ARGS =

if WITH_SMIDUMP
RPM_WITH_OPTIONS_ARGS += --with smidump
else
RPM_WITH_OPTIONS_ARGS += --without smidump
endif

if WITH_HPI
RPM_WITH_OPTIONS_ARGS += --with hpi
else
RPM_WITH_OPTIONS_ARGS += --without hpi
endif

if WITH_OPENHPI
RPM_WITH_OPTIONS_ARGS += --with openhpi
else
RPM_WITH_OPTIONS_ARGS += --without openhpi
endif

if WITH_JAVA
RPM_WITH_OPTIONS_ARGS += --with java
else
RPM_WITH_OPTIONS_ARGS += --without java
endif

