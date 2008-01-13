#           -*- OpenSAF  -*-
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

# Generates MAPI header files & miblib sources for use by applications

set -e 

if ! test -f tools/smidump/mapi_generated
then

echo "**********************************************************"
echo " Generating MAPI header & MIBLIB integration source files "
echo "**********************************************************"

export SMIPATH=$PWD/mibs:/usr/share/snmp/mibs:$PWD/samples/subagt/mibs
export ESMI_HOME=$PWD/services

cd ./tools/smidump/bin/
./esmidump_ncsmapi mqsv SAF-MSG-SVC-v7_5
./esmidump_ncsmiblib SAF-MSG-SVC-v7_5
./esmidump_ncsmapi mqsv SAF-MSG-SVC-v7_5
./esmidump_ncsmiblib  SAF-EVT-SVC-v7_5
./esmidump_ncsmapi edsv SAF-EVT-SVC-v7_5
./esmidump_ncsmapi edsv SAF-EVT-SVC-v7_5
./esmidump_ncsmiblib SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB
./esmidump_ncsmapi avsv SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB
./esmidump_ncsmiblib NCS-DTSV-MIB
./esmidump_ncsmapi dts NCS-DTSV-MIB
./esmidump_ncsmiblib SAF-LCK-SVC-v7_5
./esmidump_ncsmapi glsv SAF-LCK-SVC-v7_5
./esmidump_ncsmiblib SAF-CKPT-SVC-v7_5
./esmidump_ncsmapi cpsv SAF-CKPT-SVC-v7_5
./esmidump_ncsmiblib  IFMIB
./esmidump_ncsmapi ifsv IFMIB
./esmidump_ncsmapi ifsv IFMIB
./esmidump_ncsmapi ifsv_entp NCS-IFSV-MIB
./esmidump_ncsmapi ipxs NCS-IPXS-MIB
./esmidump_ncsmiblib  NCS-IFSV-MIB
./esmidump_ncsmapi ifsv_entp NCS-IFSV-MIB
./esmidump_ncsmiblib  NCS-IPXS-MIB
./esmidump_ncsmapi ipxs NCS-IPXS-MIB
./esmidump_ncsmiblib  NCS-VIP-MIB
./esmidump_ncsmapi vip NCS-VIP-MIB
./esmidump_ncsmiblib  NCS-IFSV-BIND-MIB
./esmidump_ncsmapi ifsv_ir NCS-IFSV-BIND-MIB
./esmidump_ncsmiblib NCS-PSR-MIB
./esmidump_ncsmiblib NCS-AVM-MIB
./esmidump_ncsmapi avm NCS-AVM-MIB
cd - 

touch tools/smidump/mapi_generated

fi
