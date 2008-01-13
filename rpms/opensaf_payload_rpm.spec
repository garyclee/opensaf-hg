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
Name: opensaf_payload
Version:1.0 
Distribution: linux
Release:5
Source: %{name}.tgz
License: High-Availability Operating Environment Software License
Vendor: OpenSAF
Packager: user@opensaf.org
Group: System
AutoReqProv: 0
Requires: tipc
Conflicts: opensaf<=1.0.5

%description
This package contains the OpenSAF Payload executables & Libraries.

#######################################
%package opensaf_payload
#######################################
Summary: OpenSAF Services Payload Blade RPM
Group: System
AutoReqProv: 0
%description opensaf_payload
This package contains the OpenSAF Services Payload executables & Libraries.

#######################################
%prep
#######################################
%setup -c -n %{name}

#######################################
%install
#######################################
cp -r opt $RPM_BUILD_ROOT 
cp -r etc $RPM_BUILD_ROOT 

###############################################################################
# NOTE:	first install will call pre(1), install, then call post(1)            #
#	erase will call preun(0), uninstall, then call postun(0)                  #
#	upgrade will call pre(2), install new, call post(2),                      #
#		then call preun(1), uninstall remaining old, and call postun(1)       #
###############################################################################
%pre 
#######################################
mkdir -p /opt/opensaf/payload/
mkdir -p /etc/opt/opensaf/

#######################################
%post 
#######################################

mkdir -p /var/opt/opensaf/log
mkdir -p /var/opt/opensaf/mdslog
mkdir -p /var/opt/opensaf/nidlog

ln -sf /sbin/reboot /etc/opt/opensaf/reboot

# if [ "%{name}" = "opensaf_payload" ]
# then
# echo "1" > /etc/opt/opensaf/init_role
# fi

cd /opt/opensaf/
ln -s /opt/opensaf/payload/lib lib

# Add an entry to /etc/profile for enabling coredump
# echo "ulimit -c 300000  # NCS_COREDUMP enable" >> /etc/profile

########################################################

## Adding the LD_LIBRARY_PATH into /etc/ld.so.conf

check=`grep "/opt/opensaf/payload/lib/" /etc/ld.so.conf `
if [ "$check" == "" ]
then
   echo "/opt/opensaf/payload/lib/" >>/etc/ld.so.conf
fi

/sbin/ldconfig

#######################################################################
# Create alias for get_ha_state and nis_scxb
#######################################################################

echo "alias 'nis_pld=/opt/opensaf/payload/scripts/nis_pld'" >> /root/.bashrc
source /root/.bashrc


#Uninstallation pre's
#######################################
%preun 
#######################################

###/etc/init.d/ncs_pb_initd stop 

#Uninstallation post: delet the softlink
#######################################
%postun
#######################################
rm -rf /opt/opensaf/payload/
rm -rf /opt/opensaf/include/
rm /opt/opensaf/lib
rm -rf /var/opt/opensaf/
rm -rf /etc/opt/opensaf/

########################################
%files 
#######################################
###############################
#Ncs Services Payload Executables 
%defattr(755,root,root)
###############################
#/opt/opensaf/payload/
#/etc/opt/opensaf/
#/var/opt/opensaf/log
/opt/opensaf/payload/bin/ncs_cpnd
/opt/opensaf/payload/bin/ncs_glnd
/opt/opensaf/payload/bin/ncs_ifnd
/opt/opensaf/payload/bin/ncs_ifsv_ip_installer
/opt/opensaf/payload/bin/ncs_mqnd
/opt/opensaf/payload/bin/ncs_nid
/opt/opensaf/payload/bin/ncs_pcap
/opt/opensaf/payload/bin/ncs_srmnd


###############################
#NCS Services public header files
%defattr(644,root,root)
###############################
###############################

/opt/opensaf/include/cpsv_papi.h
/opt/opensaf/include/dta_papi.h
/opt/opensaf/include/dts_papi.h
/opt/opensaf/include/hpl_api.h
/opt/opensaf/include/hpl_msg.h
/opt/opensaf/include/ifa_papi.h
/opt/opensaf/include/ifsv_cli_papi.h
/opt/opensaf/include/ifsv_papi.h
/opt/opensaf/include/ipxs_papi.h
/opt/opensaf/include/lfm_avm_intf.h
/opt/opensaf/include/mab_penv.h
/opt/opensaf/include/mac_papi.h
/opt/opensaf/include/mbcsv_papi.h
/opt/opensaf/include/mds_papi.h
/opt/opensaf/include/ncs_cli.h
/opt/opensaf/include/ncs_dl.h
/opt/opensaf/include/ncs_edu_pub.h
/opt/opensaf/include/ncsencdec_pub.h
/opt/opensaf/include/ncsgl_defs.h
/opt/opensaf/include/ncs_hdl_pub.h
/opt/opensaf/include/ncs_ip.h
/opt/opensaf/include/ncs_iplib.h
/opt/opensaf/include/ncs_ipv4.h
/opt/opensaf/include/ncs_ipv6.h
/opt/opensaf/include/ncs_lib.h
/opt/opensaf/include/ncs_log.h
/opt/opensaf/include/ncs_main_papi.h
/opt/opensaf/include/ncs_mda_papi.h
/opt/opensaf/include/ncs_mds_def.h
/opt/opensaf/include/ncsmiblib.h
/opt/opensaf/include/ncsmib_mem.h
/opt/opensaf/include/ncs_mib_pub.h
/opt/opensaf/include/ncs_mtbl.h
/opt/opensaf/include/ncs_mtree.h
/opt/opensaf/include/ncs_osprm.h
/opt/opensaf/include/ncspatricia.h
/opt/opensaf/include/ncs_posix_tmr.h
/opt/opensaf/include/ncs_queue.h
/opt/opensaf/include/ncs_saf.h
/opt/opensaf/include/ncs_scktprm.h
/opt/opensaf/include/ncs_sprr_papi.h
/opt/opensaf/include/ncs_stack_pub.h
/opt/opensaf/include/ncs_svd.h
/opt/opensaf/include/ncssysf_def.h
/opt/opensaf/include/ncssysf_ipc.h
/opt/opensaf/include/ncssysf_lck.h
/opt/opensaf/include/ncssysf_mem.h
/opt/opensaf/include/ncssysfpool.h
/opt/opensaf/include/ncssysf_sem.h
/opt/opensaf/include/ncssysf_tmr.h
/opt/opensaf/include/ncssysf_tsk.h
/opt/opensaf/include/ncs_tmr.h
/opt/opensaf/include/ncs_trap.h
/opt/opensaf/include/ncs_ubaid.h
/opt/opensaf/include/ncsusrbuf.h
/opt/opensaf/include/ncs_xmtre.h
/opt/opensaf/include/nid_api.h
/opt/opensaf/include/nid_err.h
/opt/opensaf/include/oac_papi.h
/opt/opensaf/include/os_defs.h
/opt/opensaf/include/psr_mib_load.h
/opt/opensaf/include/psr_papi.h
/opt/opensaf/include/rda_papi.h
/opt/opensaf/include/saAis.h
/opt/opensaf/include/saAmf.h
/opt/opensaf/include/saCkpt.h
/opt/opensaf/include/saClm.h
/opt/opensaf/include/saEvt.h
/opt/opensaf/include/SaHpi.h
/opt/opensaf/include/saLck.h
/opt/opensaf/include/saMsg.h
/opt/opensaf/include/sckt_defs.h
/opt/opensaf/include/srma_papi.h
/opt/opensaf/include/srmsv_papi.h
/opt/opensaf/include/subagt_mab.h
/opt/opensaf/include/subagt_oid_db.h
/opt/opensaf/include/sysf_ip.h
/opt/opensaf/include/ip_defs.h
/opt/opensaf/include/ncs_ipprm.h
/opt/opensaf/include/trg_defs.h

###############################
#Ncs Services SCXB Shareable libraries 
%defattr(755,root,root)
###############################
###############################

/opt/opensaf/payload/lib/libSaAmf.so
/opt/opensaf/payload/lib/libSaAmf.so.0
/opt/opensaf/payload/lib/libSaAmf.so.0.0.0
/opt/opensaf/payload/lib/libSaCkpt.so
/opt/opensaf/payload/lib/libSaCkpt.so.0
/opt/opensaf/payload/lib/libSaCkpt.so.0.0.0
/opt/opensaf/payload/lib/libSaClm.so
/opt/opensaf/payload/lib/libSaClm.so.0
/opt/opensaf/payload/lib/libSaClm.so.0.0.0
/opt/opensaf/payload/lib/libSaEvt.so
/opt/opensaf/payload/lib/libSaEvt.so.0
/opt/opensaf/payload/lib/libSaEvt.so.0.0.0
/opt/opensaf/payload/lib/libSaLck.so
/opt/opensaf/payload/lib/libSaLck.so.0
/opt/opensaf/payload/lib/libSaLck.so.0.0.0
/opt/opensaf/payload/lib/libSaMsg.so
/opt/opensaf/payload/lib/libSaMsg.so.0
/opt/opensaf/payload/lib/libSaMsg.so.0.0.0
/opt/opensaf/payload/lib/libavsv_common.so
/opt/opensaf/payload/lib/libavsv_common.so.0
/opt/opensaf/payload/lib/libavsv_common.so.0.0.0
/opt/opensaf/payload/lib/libcpsv_common.so
/opt/opensaf/payload/lib/libcpsv_common.so.0
/opt/opensaf/payload/lib/libcpsv_common.so.0.0.0
/opt/opensaf/payload/lib/libedsv_common.so
/opt/opensaf/payload/lib/libedsv_common.so.0
/opt/opensaf/payload/lib/libedsv_common.so.0.0.0
/opt/opensaf/payload/lib/libglsv_common.so
/opt/opensaf/payload/lib/libglsv_common.so.0
/opt/opensaf/payload/lib/libglsv_common.so.0.0.0
/opt/opensaf/payload/lib/libifa.so
/opt/opensaf/payload/lib/libifa.so.0
/opt/opensaf/payload/lib/libifa.so.0.0.0
/opt/opensaf/payload/lib/libifsv_common.so
/opt/opensaf/payload/lib/libifsv_common.so.0
/opt/opensaf/payload/lib/libifsv_common.so.0.0.0
/opt/opensaf/payload/lib/libipxs_subagt.so
/opt/opensaf/payload/lib/libipxs_subagt.so.0
/opt/opensaf/payload/lib/libipxs_subagt.so.0.0.0
/opt/opensaf/payload/lib/libmaa.so
/opt/opensaf/payload/lib/libmaa.so.0
/opt/opensaf/payload/lib/libmaa.so.0.0.0
/opt/opensaf/payload/lib/libmbca.so
/opt/opensaf/payload/lib/libmbca.so.0
/opt/opensaf/payload/lib/libmbca.so.0.0.0
/opt/opensaf/payload/lib/libmqsv_common.so
/opt/opensaf/payload/lib/libmqsv_common.so.0
/opt/opensaf/payload/lib/libmqsv_common.so.0.0.0
/opt/opensaf/payload/lib/libncs_core.so
/opt/opensaf/payload/lib/libncs_core.so.0
/opt/opensaf/payload/lib/libncs_core.so.0.0.0
/opt/opensaf/payload/lib/librda.so
/opt/opensaf/payload/lib/librda.so.0
/opt/opensaf/payload/lib/librda.so.0.0.0
/opt/opensaf/payload/lib/libsaf_common.so
/opt/opensaf/payload/lib/libsaf_common.so.0
/opt/opensaf/payload/lib/libsaf_common.so.0.0.0
/opt/opensaf/payload/lib/libsrma.so
/opt/opensaf/payload/lib/libsrma.so.0
/opt/opensaf/payload/lib/libsrma.so.0.0.0

###############################
#Ncs Services related SCXB conf file
###############################
%defattr(755,root,root)
 
# Startup scripts
###############################
%config /etc/opt/opensaf/chassis_id
%config /etc/opt/opensaf/slot_id
%config /etc/opt/opensaf/nodeinit.conf
/etc/opt/opensaf/tipc_reset.sh


/opt/opensaf/payload/scripts/find_pid.sh
/opt/opensaf/payload/scripts/ncs_cpnd_clean.sh
/opt/opensaf/payload/scripts/ncs_cpnd_start.sh
/opt/opensaf/payload/scripts/ncs_glnd_clean.sh
/opt/opensaf/payload/scripts/ncs_glnd_start.sh
/opt/opensaf/payload/scripts/ncs_ifnd_clean.sh
/opt/opensaf/payload/scripts/ncs_ifnd_start.sh
/opt/opensaf/payload/scripts/ncs_mqnd_clean.sh
/opt/opensaf/payload/scripts/ncs_mqnd_start.sh
/opt/opensaf/payload/scripts/ncs_srmnd_clean.sh
/opt/opensaf/payload/scripts/ncs_srmnd_start.sh
/opt/opensaf/payload/scripts/nid_tipc.sh
/opt/opensaf/payload/scripts/nis_pld
/opt/opensaf/payload/scripts/pld_cor_time
/opt/opensaf/payload/scripts/fabric_bond_conf
/opt/opensaf/payload/scripts/S31dhclnt
/opt/opensaf/payload/scripts/dhclnt_clean.sh
/opt/opensaf/payload/scripts/dhclnt_start.sh
/opt/opensaf/payload/scripts/ncs_ifsv_ip_ins_script
/opt/opensaf/payload/scripts/collect_logs.sh

