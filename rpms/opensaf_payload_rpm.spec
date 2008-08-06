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

%define lib_dir /opt/opensaf/payload/%{LIB_SUFFIX}
%define bin_dir /opt/opensaf/payload/bin
%define scripts_dir /opt/opensaf/payload/scripts
%define conf_dir /etc/opt/opensaf
%define var_dir /var/opt/opensaf
%define inc_dir /opt/opensaf/include


Summary: OpenSAF Services Payload Blade RPM
Name: opensaf_payload
Version: 2.0.0
Distribution: linux
Release:1
Source: %{name}.tgz
License: High-Availability Operating Environment Software License
Vendor: OpenSAF
Packager: user@opensaf.org
Group: System
AutoReqProv: 0
Requires: tipc

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
mkdir -p %{conf_dir}/

#######################################
%post 
#######################################

mkdir -p %{var_dir}/log
mkdir -p %{var_dir}/mdslog
mkdir -p %{var_dir}/nidlog
mkdir -p /opt/TIPC

ln -sf /sbin/reboot %{conf_dir}/reboot
ln -s %{scripts_dir}/nis_pld /etc/init.d/nis_pld

# if [ "%{name}" = "opensaf_payload" ]
# then
# echo "1" > %{conf_dir}/init_role
# fi

cd /opt/opensaf/
ln -s %{lib_dir} lib

mv %{scripts_dir}/tipc_reset.sh /opt/TIPC
#mv %{scripts_dir}/node_id %{var_dir}/node_id
#mv %{scripts_dir}/node_ha_state %{var_dir}/node_ha_state

# Add an entry to /etc/profile for enabling coredump
# echo "ulimit -c 300000  # NCS_COREDUMP enable" >> /etc/profile

########################################################

## Adding the LD_LIBRARY_PATH into /etc/ld.so.conf

check=`grep "%{lib_dir}/" /etc/ld.so.conf `
if [ "$check" == "" ]
then
   echo "%{lib_dir}/" >>/etc/ld.so.conf
fi

/sbin/ldconfig

#######################################################################
# Create alias for get_ha_state and nis_scxb
#######################################################################

echo "alias 'nis_pld=%{scripts_dir}/nis_pld'" >> /root/.bashrc
echo "alias 'saflogger=%{bin_dir}/saflogger'" >> /root/.bashrc

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
rm -rf %{inc_dir}/
rm /opt/opensaf/lib
rm -rf %{var_dir}/
rm -rf %{conf_dir}/
unlink /etc/init.d/nis_pld

########################################
%files 
#######################################
###############################
#Ncs Services Payload Executables 
%defattr(755,root,root)
###############################
#/opt/opensaf/payload/
#%{conf_dir}/
#%{var_dir}/log
%{bin_dir}/ncs_cpnd
%{bin_dir}/ncs_glnd
%{bin_dir}/ncs_ifnd
%{bin_dir}/ncs_ifsv_ip_installer
%{bin_dir}/ncs_mqnd
%{bin_dir}/ncs_nid
%{bin_dir}/ncs_pcap
%{bin_dir}/ncs_srmnd
%{bin_dir}/saflogger

###############################
#NCS Services public header files
%defattr(644,root,root)
###############################
###############################

%{inc_dir}/cpsv_papi.h
%{inc_dir}/dta_papi.h
%{inc_dir}/dts_papi.h
%{inc_dir}/hpl_api.h
%{inc_dir}/hpl_msg.h
%{inc_dir}/ifa_papi.h
%{inc_dir}/ifsv_cli_papi.h
%{inc_dir}/ifsv_papi.h
%{inc_dir}/ipxs_papi.h
%{inc_dir}/lfm_avm_intf.h
%{inc_dir}/mab_penv.h
%{inc_dir}/mac_papi.h
%{inc_dir}/mbcsv_papi.h
%{inc_dir}/mds_papi.h
%{inc_dir}/ncs_cli.h
%{inc_dir}/ncs_dl.h
%{inc_dir}/ncs_edu_pub.h
%{inc_dir}/ncsencdec_pub.h
%{inc_dir}/ncsgl_defs.h
%{inc_dir}/ncs_hdl_pub.h
%{inc_dir}/ncs_ip.h
%{inc_dir}/ncs_iplib.h
%{inc_dir}/ncs_ipv4.h
%{inc_dir}/ncs_ipv6.h
%{inc_dir}/ncs_lib.h
%{inc_dir}/ncs_log.h
%{inc_dir}/ncs_main_papi.h
%{inc_dir}/ncs_mda_papi.h
%{inc_dir}/ncs_mds_def.h
%{inc_dir}/ncsmiblib.h
%{inc_dir}/ncsmib_mem.h
%{inc_dir}/ncs_mib_pub.h
%{inc_dir}/ncs_mtbl.h
%{inc_dir}/ncs_mtree.h
%{inc_dir}/ncs_osprm.h
%{inc_dir}/ncspatricia.h
%{inc_dir}/ncs_posix_tmr.h
%{inc_dir}/ncs_queue.h
%{inc_dir}/ncs_saf.h
%{inc_dir}/ncs_scktprm.h
%{inc_dir}/ncs_sprr_papi.h
%{inc_dir}/ncs_stack_pub.h
%{inc_dir}/ncs_svd.h
%{inc_dir}/ncssysf_def.h
%{inc_dir}/ncssysf_ipc.h
%{inc_dir}/ncssysf_lck.h
%{inc_dir}/ncssysf_mem.h
%{inc_dir}/ncssysfpool.h
%{inc_dir}/ncssysf_sem.h
%{inc_dir}/ncssysf_tmr.h
%{inc_dir}/ncssysf_tsk.h
%{inc_dir}/ncs_tmr.h
%{inc_dir}/ncs_trap.h
%{inc_dir}/ncs_ubaid.h
%{inc_dir}/ncsusrbuf.h
%{inc_dir}/ncs_xmtre.h
%{inc_dir}/nid_api.h
%{inc_dir}/nid_err.h
%{inc_dir}/oac_papi.h
%{inc_dir}/os_defs.h
%{inc_dir}/psr_mib_load.h
%{inc_dir}/psr_papi.h
%{inc_dir}/rda_papi.h
%{inc_dir}/saAis.h
%{inc_dir}/saAmf.h
%{inc_dir}/saCkpt.h
%{inc_dir}/saClm.h
%{inc_dir}/saEvt.h
%{inc_dir}/SaHpi.h
%{inc_dir}/saLck.h
%{inc_dir}/saMsg.h
%{inc_dir}/sckt_defs.h
%{inc_dir}/srma_papi.h
%{inc_dir}/srmsv_papi.h
%{inc_dir}/subagt_mab.h
%{inc_dir}/subagt_oid_db.h
%{inc_dir}/sysf_ip.h
%{inc_dir}/ip_defs.h
%{inc_dir}/ncs_ipprm.h
%{inc_dir}/trg_defs.h

###############################
#Ncs Services SCXB Shareable libraries 
%defattr(755,root,root)
###############################
###############################

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
%{lib_dir}/libSaLog.so
%{lib_dir}/libSaLog.so.0
%{lib_dir}/libSaLog.so.0.0.0
%{lib_dir}/libSaMsg.so
%{lib_dir}/libSaMsg.so.0
%{lib_dir}/libSaMsg.so.0.0.0
%{lib_dir}/libavsv_common.so
%{lib_dir}/libavsv_common.so.0
%{lib_dir}/libavsv_common.so.0.0.0
%{lib_dir}/libcpsv_common.so
%{lib_dir}/libcpsv_common.so.0
%{lib_dir}/libcpsv_common.so.0.0.0
%{lib_dir}/libedsv_common.so
%{lib_dir}/libedsv_common.so.0
%{lib_dir}/libedsv_common.so.0.0.0
%{lib_dir}/libglsv_common.so
%{lib_dir}/libglsv_common.so.0
%{lib_dir}/libglsv_common.so.0.0.0
%{lib_dir}/libifa.so
%{lib_dir}/libifa.so.0
%{lib_dir}/libifa.so.0.0.0
%{lib_dir}/libifsv_common.so
%{lib_dir}/libifsv_common.so.0
%{lib_dir}/libifsv_common.so.0.0.0
%{lib_dir}/libmaa.so
%{lib_dir}/libmaa.so.0
%{lib_dir}/libmaa.so.0.0.0
%{lib_dir}/libmbca.so
%{lib_dir}/libmbca.so.0
%{lib_dir}/libmbca.so.0.0.0
%{lib_dir}/libmqsv_common.so
%{lib_dir}/libmqsv_common.so.0
%{lib_dir}/libmqsv_common.so.0.0.0
%{lib_dir}/libncs_core.so
%{lib_dir}/libncs_core.so.0
%{lib_dir}/libncs_core.so.0.0.0
%{lib_dir}/librda.so
%{lib_dir}/librda.so.0
%{lib_dir}/librda.so.0.0.0
%{lib_dir}/libsaf_common.so
%{lib_dir}/libsaf_common.so.0
%{lib_dir}/libsaf_common.so.0.0.0
%{lib_dir}/libsrma.so
%{lib_dir}/libsrma.so.0
%{lib_dir}/libsrma.so.0.0.0

 
###############################
# Config files 
###############################
%defattr(644,root,root)

%config %{conf_dir}/chassis_id
%config %{conf_dir}/slot_id
%config %{conf_dir}/nodeinit.conf

########################################
# Variable Data files & Startup scripts 
########################################
%defattr(755,root,root)

%{scripts_dir}/tipc_reset.sh


%{scripts_dir}/find_pid.sh
%{scripts_dir}/ncs_pcap_clean.sh
%{scripts_dir}/ncs_pcap_start.sh
%{scripts_dir}/ncs_cpnd_clean.sh
%{scripts_dir}/ncs_cpnd_start.sh
%{scripts_dir}/ncs_glnd_clean.sh
%{scripts_dir}/ncs_glnd_start.sh
%{scripts_dir}/ncs_ifnd_clean.sh
%{scripts_dir}/ncs_ifnd_start.sh
%{scripts_dir}/ncs_mqnd_clean.sh
%{scripts_dir}/ncs_mqnd_start.sh
%{scripts_dir}/ncs_srmnd_clean.sh
%{scripts_dir}/ncs_srmnd_start.sh
%{scripts_dir}/nid_tipc.sh
%{scripts_dir}/nis_pld
%{scripts_dir}/pld_cor_time
%{scripts_dir}/fabric_bond_conf
%{scripts_dir}/S31dhclnt
%{scripts_dir}/dhclnt_clean.sh
%{scripts_dir}/dhclnt_start.sh
%{scripts_dir}/ncs_ifsv_ip_ins_script
%{scripts_dir}/collect_logs.sh

