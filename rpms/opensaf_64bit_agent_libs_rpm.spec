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

Summary: OpenSAF Services Payload Blade RPM
Name: opensaf_64bit_agent_libs
Version:1.0 
Distribution: linux
Release:5
Source: %{name}.tgz
License: High-Availability Operating Environment Software License
Vendor: OpenSAF
Packager: users@opensaf.org
Group: System
AutoReqProv: 0

%description
This package contains the OpenSAF Payload Libraries for lib64.

########################################
#%package opensaf_payload_64bit_libs
########################################
#Summary: OpenSAF Services Payload Blade RPM
#Group: System
#AutoReqProv: 0
#%description opensaf_payload_64bit_libs
#This package contains the OpenSAF Services Payload executables & Libraries.

#######################################
%prep
#######################################
%setup -c -n %{name}

#######################################
%install
#######################################
cp -r opt $RPM_BUILD_ROOT

###############################################################################
# NOTE:	first install will call pre(1), install, then call post(1)            #
#	erase will call preun(0), uninstall, then call postun(0)                  #
#	upgrade will call pre(2), install new, call post(2),                      #
#		then call preun(1), uninstall remaining old, and call postun(1)       #
###############################################################################
%pre 
#######################################
mkdir -p /opt/opensaf/payload/

#######################################
%post 
#######################################

ln -sf /sbin/reboot /etc/opt/opensaf/reboot

check=`grep "/opt/opensaf/payload/lib64/" /etc/ld.so.conf `
if [ "$check" == "" ]
then
   echo "/opt/opensaf/payload/lib64/" >>/etc/ld.so.conf
fi


/sbin/ldconfig


#Uninstallation pre's
#######################################
%preun 
#######################################

###/etc/init.d/ncs_pb_initd stop 

#Uninstallation post: delet the softlink
#######################################
%postun
#######################################


########################################
%files 
#######################################

# adding 64bit agent libraries

/opt/opensaf/payload/lib64/libncs_core.so
/opt/opensaf/payload/lib64/libncs_core.so.0
/opt/opensaf/payload/lib64/libncs_core.so.0.0.0
/opt/opensaf/payload/lib64/libSaAmf.so
/opt/opensaf/payload/lib64/libSaAmf.so.0
/opt/opensaf/payload/lib64/libSaAmf.so.0.0.0
/opt/opensaf/payload/lib64/libSaCkpt.so
/opt/opensaf/payload/lib64/libSaCkpt.so.0
/opt/opensaf/payload/lib64/libSaCkpt.so.0.0.0
/opt/opensaf/payload/lib64/libSaClm.so
/opt/opensaf/payload/lib64/libSaClm.so.0
/opt/opensaf/payload/lib64/libSaClm.so.0.0.0
/opt/opensaf/payload/lib64/libSaEvt.so
/opt/opensaf/payload/lib64/libSaEvt.so.0
/opt/opensaf/payload/lib64/libSaEvt.so.0.0.0
/opt/opensaf/payload/lib64/libSaLck.so
/opt/opensaf/payload/lib64/libSaLck.so.0
/opt/opensaf/payload/lib64/libSaLck.so.0.0.0
/opt/opensaf/payload/lib64/libSaMsg.so
/opt/opensaf/payload/lib64/libSaMsg.so.0
/opt/opensaf/payload/lib64/libSaMsg.so.0.0.0
/opt/opensaf/payload/lib64/libmaa.so
/opt/opensaf/payload/lib64/libmaa.so.0
/opt/opensaf/payload/lib64/libmaa.so.0.0.0
/opt/opensaf/payload/lib64/libmbca.so
/opt/opensaf/payload/lib64/libmbca.so.0
/opt/opensaf/payload/lib64/libmbca.so.0.0.0
/opt/opensaf/payload/lib64/libsrma.so
/opt/opensaf/payload/lib64/libsrma.so.0
/opt/opensaf/payload/lib64/libsrma.so.0.0.0
/opt/opensaf/payload/lib64/libifa.so
/opt/opensaf/payload/lib64/libifa.so.0
/opt/opensaf/payload/lib64/libifa.so.0.0.0
/opt/opensaf/payload/lib64/libsaf_common.so
/opt/opensaf/payload/lib64/libsaf_common.so.0
/opt/opensaf/payload/lib64/libsaf_common.so.0.0.0
/opt/opensaf/payload/lib64/libavsv_common.so
/opt/opensaf/payload/lib64/libavsv_common.so.0
/opt/opensaf/payload/lib64/libavsv_common.so.0.0.0
/opt/opensaf/payload/lib64/libcpsv_common.so
/opt/opensaf/payload/lib64/libcpsv_common.so.0
/opt/opensaf/payload/lib64/libcpsv_common.so.0.0.0
/opt/opensaf/payload/lib64/libmqsv_common.so
/opt/opensaf/payload/lib64/libmqsv_common.so.0
/opt/opensaf/payload/lib64/libmqsv_common.so.0.0.0
/opt/opensaf/payload/lib64/libedsv_common.so
/opt/opensaf/payload/lib64/libedsv_common.so.0
/opt/opensaf/payload/lib64/libedsv_common.so.0.0.0
/opt/opensaf/payload/lib64/libifsv_common.so
/opt/opensaf/payload/lib64/libifsv_common.so.0
/opt/opensaf/payload/lib64/libifsv_common.so.0.0.0



