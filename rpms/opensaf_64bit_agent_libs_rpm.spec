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

%define lib_dir /opt/opensaf/payload/lib64

Summary: OpenSAF Services Payload Blade RPM
Name: opensaf_64bit_agent_libs
Version: 1.2.2
Distribution: linux
Release:1
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

cd /opt/opensaf/
ln -s %{lib_dir} lib

check=`grep "%{lib_dir}/" /etc/ld.so.conf `
if [ "$check" == "" ]
then
   echo "%{lib_dir}/" >>/etc/ld.so.conf
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

%{lib_dir}/libncs_core.so
%{lib_dir}/libncs_core.so.0
%{lib_dir}/libncs_core.so.0.0.0
%{lib_dir}/libSaAmf.so
%{lib_dir}/libSaAmf.so.0
%{lib_dir}/libSaAmf.so.0.0.0
%{lib_dir}/libSaCkpt.so
%{lib_dir}/libSaCkpt.so.0
%{lib_dir}/libSaCkpt.so.0.0.0
%{lib_dir}/libSaClm.so
%{lib_dir}/libSaClm.so.0
%{lib_dir}/libSaClm.so.0.0.0
%{lib_dir}/libSaEvt.so
%{lib_dir}/libSaEvt.so.0
%{lib_dir}/libSaEvt.so.0.0.0
%{lib_dir}/libSaLck.so
%{lib_dir}/libSaLck.so.0
%{lib_dir}/libSaLck.so.0.0.0
%{lib_dir}/libSaMsg.so
%{lib_dir}/libSaMsg.so.0
%{lib_dir}/libSaMsg.so.0.0.0
%{lib_dir}/libmaa.so
%{lib_dir}/libmaa.so.0
%{lib_dir}/libmaa.so.0.0.0
%{lib_dir}/libmbca.so
%{lib_dir}/libmbca.so.0
%{lib_dir}/libmbca.so.0.0.0
%{lib_dir}/libsrma.so
%{lib_dir}/libsrma.so.0
%{lib_dir}/libsrma.so.0.0.0
%{lib_dir}/libifa.so
%{lib_dir}/libifa.so.0
%{lib_dir}/libifa.so.0.0.0
%{lib_dir}/libsaf_common.so
%{lib_dir}/libsaf_common.so.0
%{lib_dir}/libsaf_common.so.0.0.0
%{lib_dir}/libavsv_common.so
%{lib_dir}/libavsv_common.so.0
%{lib_dir}/libavsv_common.so.0.0.0
%{lib_dir}/libcpsv_common.so
%{lib_dir}/libcpsv_common.so.0
%{lib_dir}/libcpsv_common.so.0.0.0
%{lib_dir}/libmqsv_common.so
%{lib_dir}/libmqsv_common.so.0
%{lib_dir}/libmqsv_common.so.0.0.0
%{lib_dir}/libedsv_common.so
%{lib_dir}/libedsv_common.so.0
%{lib_dir}/libedsv_common.so.0.0.0
%{lib_dir}/libifsv_common.so
%{lib_dir}/libifsv_common.so.0
%{lib_dir}/libifsv_common.so.0.0.0



