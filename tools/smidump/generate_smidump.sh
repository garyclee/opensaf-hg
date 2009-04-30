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
# Author(s): Emerson Network Power
#

# Build smidump tool. Once.

set -e

if [ ! -f ./tools/smidump/bin/smidump ]; then
        echo "**********************************"
        echo "Building SMIDUMP tool"
        echo "**********************************"

        # Making sure we are not cross-compiling smidump
        unset CFLAGS
        unset CXXFLAGS
        unset CC
        unset CXX
        unset LDFLAGS
        unset AS
        unset AR
        unset CPP
        unset LD
        unset NM
        unset OBJCOPY
        unset OBJDUMP
        unset RANLIB
        unset READELF
        unset STRIP
        unset CONFIG_SITE
        unset PKG_CONFIG_PATH

	cd ./tools/smidump/src
	./configure --enable-static --disable-shared
	make
	cd -
        mkdir -p ./tools/smidump/bin
	cp ./tools/smidump/src/tools/smidump ./tools/smidump/bin/
else
        echo "Nothing to be done for 'build_smidump'"
fi
