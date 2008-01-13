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


# Generates files for integration with the subagent and PSSV accordingly

set -e 

if ! test -f tools/smidump/files_generated
then

echo "*****************************************************"
echo "Generating SNMP subagent & PSSV Integration files    "
echo "*****************************************************"

export SMIPATH=$PWD/mibs:/usr/share/snmp/mibs:$PWD/samples/subagt/mibs
export ESMI_HOME=$PWD/services

cd ./tools/smidump/bin/
./esmidump_pssv avsv SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB
./esmidump_pssv dts NCS-DTSV-MIB
./esmidump_newncssubagt psr NCS-PSR-MIB
./esmidump_newncssubagt glsv SAF-LCK-SVC-v7_5
./esmidump_newncssubagt avsv SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB
./esmidump_newncssubagt cpsv SAF-CKPT-SVC-v7_5  
./esmidump_newncssubagt ifsv IFMIB
./esmidump_newncssubagt ifsv_entp NCS-IFSV-MIB
./esmidump_newncssubagt ifsv_ir NCS-IFSV-BIND-MIB
./esmidump_newncssubagt ipxs NCS-IPXS-MIB
./esmidump_newncssubagt dtsv NCS-DTSV-MIB
./esmidump_newncssubagt avm NCS-AVM-MIB
./esmidump_newncssubagt edsv SAF-EVT-SVC-v7_5
./esmidump_newncssubagt mqsv SAF-MSG-SVC-v7_5
./esmidump_pssv avm NCS-AVM-MIB
cd - 


touch tools/smidump/files_generated

fi
