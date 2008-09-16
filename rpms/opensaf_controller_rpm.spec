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

%define lib_dir /opt/opensaf/controller/%{LIB_SUFFIX}
%define bin_dir /opt/opensaf/controller/bin
%define scripts_dir /opt/opensaf/controller/scripts
%define conf_dir /etc/opt/opensaf
%define var_dir /var/opt/opensaf
%define inc_dir /opt/opensaf/include
 
Summary: OpenSAF Services Controller Blade RPM
Name: opensaf_controller
Version: 2.0.0
Distribution: linux
Release:1
Source: %{name}.tgz
License: High-Availability Operating Environment Software License
Vendor: OpenSAF
Packager: user@opensaf.org
Group: System
AutoReqProv: 0
Requires: net-snmp=5.4, tipc, xerces=2.7.0
%description
This package contains the Open SAF  Controller executables & Libraries.

#######################################
%package opensaf_controller
#######################################
Summary: OpenSAF Services Controller Blade RPM
Group: System
AutoReqProv: 0
%description opensaf_controller
This package contains the OpenSAF Services Controller executables & Libraries.

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
mkdir -p /opt/opensaf/controller/
mkdir -p %{conf_dir}/

#######################################
%post 
#######################################

mkdir -p /opt/TIPC
mkdir -p /usr/share/snmp/

cp %{conf_dir}/ncsSnmpSubagt.conf /usr/share/snmp/
mkdir -p %{var_dir}/mdslog
mkdir -p %{var_dir}/nidlog
mkdir -p %{var_dir}/saflog

########################################
####  Create soft links 
#######################################
mkdir -p /repl_opensaf/config
mkdir -p /repl_opensaf/pssv_store
mkdir -p /repl_opensaf/log

mv %{scripts_dir}/node_ha_state %{var_dir}/
chmod 764 %{var_dir}/node_ha_state

mv %{conf_dir}/NCSSystemBOM.xml /repl_opensaf/config/NCSSystemBOM.xml
cd %{conf_dir}/
ln -s /repl_opensaf/config/NCSSystemBOM.xml NCSSystemBOM.xml

mv %{conf_dir}/ValidationConfig.xml   /repl_opensaf/config/ValidationConfig.xml
cd %{conf_dir}/
ln -s /repl_opensaf/config/ValidationConfig.xml   ValidationConfig.xml

mv %{conf_dir}/ValidationConfig.xsd   /repl_opensaf/config/ValidationConfig.xsd
cd %{conf_dir}/
ln -s /repl_opensaf/config/ValidationConfig.xsd   ValidationConfig.xsd

mv /etc/opt/opensaf/AppConfig.xsd   /repl_opensaf/config/AppConfig.xsd
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/AppConfig.xsd   AppConfig.xsd

mv %{conf_dir}/NCSConfig.xsd   /repl_opensaf/config/NCSConfig.xsd
cd %{conf_dir}/
ln -s /repl_opensaf/config/NCSConfig.xsd   NCSConfig.xsd

mv %{conf_dir}/vipsample.conf    /repl_opensaf/config/vipsample.conf 
cd %{conf_dir}/
ln -s /repl_opensaf/config/vipsample.conf    vipsample.conf 

mv %{conf_dir}/subagt_lib_conf   /repl_opensaf/config/subagt_lib_conf
cd %{conf_dir}/
ln -s /repl_opensaf/config/subagt_lib_conf   subagt_lib_conf

mv %{conf_dir}/cli_cefslib_conf    /repl_opensaf/config/cli_cefslib_conf 
cd %{conf_dir}/
ln -s /repl_opensaf/config/cli_cefslib_conf    cli_cefslib_conf 

cd /opt/opensaf/
ln -s %{lib_dir} lib

cd %{var_dir} 
ln -s /repl_opensaf/log  log   

cd %{var_dir}
ln -s /repl_opensaf/pssv_store   pssv_store   

ln -sf /sbin/reboot %{conf_dir}/reboot

# LSB related changes
mv %{scripts_dir}/tipc_reset.sh /opt/TIPC/
mv %{scripts_dir}/pssv_spcn_list %{var_dir}

#mv %{scripts_dir}/node_id %{var_dir}

ln -s %{scripts_dir}/nis_scxb /etc/init.d/nis_scxb

########################################
####  Create/Define CLI users' Groups 
########################################

############################################################
### Create a group for CLI users with superuser permissions
############################################################

#Delete group first if it still exists

groupadd -g 9001 ncs_cli_superuser
if [ $? -eq 1 ] ; then
   groupdel ncs_cli_superuser
   if [ $? -eq 1 ] ; then
      echo "CLI user group creation failed, Please check your group settings"
   else 
   groupadd -g 9001 ncs_cli_superuser
   echo "group ncs_cli_superuser is created with gid 9001"
   fi
fi

#################################################################
### Create a group for CLI users with administrative privileges
#################################################################

groupadd -g 9002 ncs_cli_admin
if [ $? -eq 1 ] ; then
   groupdel ncs_cli_admin
   if [ $? -eq 1 ] ; then
      echo "CLI user group creation failed, Please check your group settings"
   else 
   groupadd -g 9002 ncs_cli_admin
   echo "group ncs_cli_admin is created with gid 9002"
   fi
fi

########################################################
### Create a group for CLI users with view permissions
########################################################

groupadd -g 9003 ncs_cli_viewer
if [ $? -eq 1 ] ; then
   groupdel ncs_cli_viewer
   if [ $? -eq 1 ] ; then
      echo "CLI user group creation failed, Please check your group settings"
   else 
   groupadd -g 9003 ncs_cli_viewer
   echo "group ncs_cli_viewer is created with gid 9003"
   fi
fi


#######################################################################
#add the default 'opensaf' user as part of the 'ncs_cli_viewer' group.
#######################################################################
useradd -g ncs_cli_viewer -d /home/opensaf -m -p abPP2vhKRsQC. opensaf

if [ $? -eq 1 ] ; then
   echo "useradd -G ncs_cli_viewer opensaf : failed"
fi

#######################################################################
#add the root as part of the 'ncs_cli_superuser' group.
#######################################################################

usermod -G ncs_cli_superuser root
if [ $? -eq 1 ] ;
then
   echo "usermod -G ncs_cli_superuser root : failed"
fi

#######################################################################
# Create alias for get_ha_state and nis_scxb
#######################################################################

echo "alias 'get_ha_state=%{scripts_dir}/get_ha_state'" >> /root/.bashrc
echo "alias 'nis_scxb=%{scripts_dir}/nis_scxb'" >> /root/.bashrc
echo "alias 'saflogger=%{bin_dir}/saflogger'" >> /root/.bashrc

source /root/.bashrc

########################################################

## Adding the LD_LIBRARY_PATH into /etc/ld.so.conf

check=`grep "/opt/opensaf/controller/lib/" /etc/ld.so.conf `
if [ "$check" == "" ]
then
   echo "%{lib_dir}/" >>/etc/ld.so.conf
fi

/sbin/ldconfig


#Uninstallation pre's
#######################################
%preun 
#######################################

#/etc/init.d/nis_scxb stop 

#Uninstallation post:
#######################################
%postun
#######################################

userdel -rf opensaf
groupdel ncs_cli_superuser
groupdel ncs_cli_admin
groupdel ncs_cli_viewer

#Delete our files
rm -rf /opt/opensaf/controller/
rm -rf %{inc_dir}/
rm /opt/opensaf/lib
rm -rf %{conf_dir}/
rm -rf %{var_dir}/
rm -rf /repl_opensaf/
unlink /etc/init.d/nis_scxb

########################################
%files 
#######################################
###############################
#Ncs Services Controller Executables 
%defattr(755,root,root)
###############################
#/opt/opensaf/controller/
#%{conf_dir}/
#%{var_dir}/
#/repl_opensaf/
%{bin_dir}/ncs_cli_maa
%{bin_dir}/ncs_cpd
%{bin_dir}/ncs_cpnd
%{bin_dir}/ncs_dts
%{bin_dir}/ncs_eds
%{bin_dir}/ncs_gld
%{bin_dir}/ncs_glnd
#%{bin_dir}/ncs_hisv 
%{bin_dir}/ncs_ifd
%{bin_dir}/ncs_ifnd
%{bin_dir}/ncs_ifsv_ip_installer
%{bin_dir}/ncs_mas
%{bin_dir}/ncs_mqd
%{bin_dir}/ncs_mqnd
%{bin_dir}/ncs_nid
%{bin_dir}/ncs_psr
%{bin_dir}/ncs_rde
%{bin_dir}/ncs_scap
%{bin_dir}/ncs_snmp_subagt
%{bin_dir}/ncs_srmnd
%{bin_dir}/ncs_vds
%{bin_dir}/ncs_pdrbd
%{bin_dir}/opensaf_saflogd
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
%{inc_dir}/saLog.h
%{inc_dir}/saMsg.h
%{inc_dir}/saNtf.h
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
%{lib_dir}/libavm_logstr.so
%{lib_dir}/libavm_logstr.so.2
%{lib_dir}/libavm_logstr.so.2.0.0
%{lib_dir}/libavm_pssv.so
%{lib_dir}/libavm_pssv.so.0
%{lib_dir}/libavm_pssv.so.0.0.0
%{lib_dir}/libavm_subagt.so
%{lib_dir}/libavm_subagt.so.0
%{lib_dir}/libavm_subagt.so.0.0.0
%{lib_dir}/libavsv_clicef.so
%{lib_dir}/libavsv_clicef.so.0
%{lib_dir}/libavsv_clicef.so.0.0.0
%{lib_dir}/libavsv_common.so
%{lib_dir}/libavsv_common.so.0
%{lib_dir}/libavsv_common.so.0.0.0
%{lib_dir}/libavsv_logstr.so
%{lib_dir}/libavsv_logstr.so.2
%{lib_dir}/libavsv_logstr.so.2.0.0
%{lib_dir}/libavsv_pssv.so
%{lib_dir}/libavsv_pssv.so.0
%{lib_dir}/libavsv_pssv.so.0.0.0
%{lib_dir}/libavsv_subagt.so
%{lib_dir}/libavsv_subagt.so.0
%{lib_dir}/libavsv_subagt.so.0.0.0
%{lib_dir}/libbam_logstr.so
%{lib_dir}/libbam_logstr.so.2
%{lib_dir}/libbam_logstr.so.2.0.0
%{lib_dir}/libcli_logstr.so
%{lib_dir}/libcli_logstr.so.2
%{lib_dir}/libcli_logstr.so.2.0.0
%{lib_dir}/libcpsv_common.so
%{lib_dir}/libcpsv_common.so.0
%{lib_dir}/libcpsv_common.so.0.0.0
%{lib_dir}/libcpsv_logstr.so
%{lib_dir}/libcpsv_logstr.so.3
%{lib_dir}/libcpsv_logstr.so.3.0.0
%{lib_dir}/libcpsv_subagt.so
%{lib_dir}/libcpsv_subagt.so.0
%{lib_dir}/libcpsv_subagt.so.0.0.0
%{lib_dir}/libdts_clicef.so
%{lib_dir}/libdts_clicef.so.0
%{lib_dir}/libdts_clicef.so.0.0.0
%{lib_dir}/libdts_logstr.so
%{lib_dir}/libdts_logstr.so.2
%{lib_dir}/libdts_logstr.so.2.0.0
%{lib_dir}/libdts_pssv.so
%{lib_dir}/libdts_pssv.so.0
%{lib_dir}/libdts_pssv.so.0.0.0
%{lib_dir}/libdtsv_subagt.so
%{lib_dir}/libdtsv_subagt.so.0
%{lib_dir}/libdtsv_subagt.so.0.0.0
%{lib_dir}/libedsv_common.so
%{lib_dir}/libedsv_common.so.0
%{lib_dir}/libedsv_common.so.0.0.0
%{lib_dir}/libedsv_logstr.so
%{lib_dir}/libedsv_logstr.so.2
%{lib_dir}/libedsv_logstr.so.2.0.0
%{lib_dir}/libedsv_subagt.so
%{lib_dir}/libedsv_subagt.so.0
%{lib_dir}/libedsv_subagt.so.0.0.0
%{lib_dir}/libglsv_common.so
%{lib_dir}/libglsv_common.so.0
%{lib_dir}/libglsv_common.so.0.0.0
%{lib_dir}/libglsv_logstr.so
%{lib_dir}/libglsv_logstr.so.3
%{lib_dir}/libglsv_logstr.so.3.0.0
%{lib_dir}/libglsv_subagt.so
%{lib_dir}/libglsv_subagt.so.0
%{lib_dir}/libglsv_subagt.so.0.0.0
%{lib_dir}/libhpl.so
%{lib_dir}/libhpl.so.0
%{lib_dir}/libhpl.so.0.0.0
#%{lib_dir}/libhisv_logstr.so
#%{lib_dir}/libhisv_logstr.so.2 
#%{lib_dir}/libhisv_logstr.so.2.0.0 
%{lib_dir}/libifa.so
%{lib_dir}/libifa.so.0
%{lib_dir}/libifa.so.0.0.0
%{lib_dir}/libifsv_bind_subagt.so
%{lib_dir}/libifsv_bind_subagt.so.0
%{lib_dir}/libifsv_bind_subagt.so.0.0.0
%{lib_dir}/libifsv_clicef.so
%{lib_dir}/libifsv_clicef.so.0
%{lib_dir}/libifsv_clicef.so.0.0.0
%{lib_dir}/libifsv_common.so
%{lib_dir}/libifsv_common.so.0
%{lib_dir}/libifsv_common.so.0.0.0
%{lib_dir}/libifsv_entp_subagt.so
%{lib_dir}/libifsv_entp_subagt.so.0
%{lib_dir}/libifsv_entp_subagt.so.0.0.0
%{lib_dir}/libifsv_logstr.so
%{lib_dir}/libifsv_logstr.so.2
%{lib_dir}/libifsv_logstr.so.2.0.0
%{lib_dir}/libifsv_subagt.so
%{lib_dir}/libifsv_subagt.so.0
%{lib_dir}/libifsv_subagt.so.0.0.0
%{lib_dir}/libipxs_subagt.so
%{lib_dir}/libipxs_subagt.so.0
%{lib_dir}/libipxs_subagt.so.0.0.0
#%{lib_dir}/liblfm_avm_intf.so 
#%{lib_dir}/liblfm_avm_intf.so.0 
#%{lib_dir}/liblfm_avm_intf.so.0.0.0 
%{lib_dir}/libmaa.so
%{lib_dir}/libmaa.so.0
%{lib_dir}/libmaa.so.0.0.0
%{lib_dir}/libmab_logstr.so
%{lib_dir}/libmab_logstr.so.3
%{lib_dir}/libmab_logstr.so.3.0.0
%{lib_dir}/libmbca.so
%{lib_dir}/libmbca.so.0
%{lib_dir}/libmbca.so.0.0.0
%{lib_dir}/libmbcsv_logstr.so
%{lib_dir}/libmbcsv_logstr.so.2
%{lib_dir}/libmbcsv_logstr.so.2.0.0
%{lib_dir}/libmqsv_common.so
%{lib_dir}/libmqsv_common.so.0
%{lib_dir}/libmqsv_common.so.0.0.0
%{lib_dir}/libmqsv_logstr.so
%{lib_dir}/libmqsv_logstr.so.3
%{lib_dir}/libmqsv_logstr.so.3.0.0
%{lib_dir}/libmqsv_subagt.so
%{lib_dir}/libmqsv_subagt.so.0
%{lib_dir}/libmqsv_subagt.so.0.0.0
%{lib_dir}/libncs_core.so
%{lib_dir}/libncs_core.so.0
%{lib_dir}/libncs_core.so.0.0.0
%{lib_dir}/libncs_snmp_subagt.so
%{lib_dir}/libncs_snmp_subagt.so.0
%{lib_dir}/libncs_snmp_subagt.so.0.0.0
%{lib_dir}/libpsr_clicef.so
%{lib_dir}/libpsr_clicef.so.0
%{lib_dir}/libpsr_clicef.so.0.0.0
%{lib_dir}/libpssv_logstr.so
%{lib_dir}/libpssv_logstr.so.4
%{lib_dir}/libpssv_logstr.so.4.0.0
%{lib_dir}/libpssv_subagt.so
%{lib_dir}/libpssv_subagt.so.0
%{lib_dir}/libpssv_subagt.so.0.0.0
%{lib_dir}/librda.so
%{lib_dir}/librda.so.0
%{lib_dir}/librda.so.0.0.0
%{lib_dir}/librde_logstr.so
%{lib_dir}/librde_logstr.so.2
%{lib_dir}/librde_logstr.so.2.0.0
%{lib_dir}/libsaf_common.so
%{lib_dir}/libsaf_common.so.0
%{lib_dir}/libsaf_common.so.0.0.0
%{lib_dir}/libsrma.so
%{lib_dir}/libsrma.so.0
%{lib_dir}/libsrma.so.0.0.0
%{lib_dir}/libsrmsv_logstr.so
%{lib_dir}/libsrmsv_logstr.so.3
%{lib_dir}/libsrmsv_logstr.so.3.0.0
%{lib_dir}/libsubagt_logstr.so
%{lib_dir}/libsubagt_logstr.so.2
%{lib_dir}/libsubagt_logstr.so.2.0.0
%{lib_dir}/libvds_logstr.so
%{lib_dir}/libvds_logstr.so.3
%{lib_dir}/libvds_logstr.so.3.0.0
%{lib_dir}/libpdrbd_logstr.so
%{lib_dir}/libpdrbd_logstr.so.2
%{lib_dir}/libpdrbd_logstr.so.2.0.0

###############################
# Config Files 
###############################
%defattr(644,root,root)
 
%config %{conf_dir}/NCSConfig.xsd
%config /etc/opt/opensaf/AppConfig.xsd
%config %{conf_dir}/NCSSystemBOM.xml
%config %{conf_dir}/ValidationConfig.xml
%config %{conf_dir}/ValidationConfig.xsd
%config %{conf_dir}/chassis_id
%config %{conf_dir}/cli_cefslib_conf
%config %{conf_dir}/ncsSnmpSubagt.conf
%config %{conf_dir}/pssv_lib_conf
%config %{conf_dir}/rde.conf
%config %{conf_dir}/slot_id
%config %{conf_dir}/subagt_lib_conf
%config %{conf_dir}/vipsample.conf
%config %{conf_dir}/.drbd_sync_state_0
%config %{conf_dir}/.drbd_sync_state_1
%config %{conf_dir}/.drbd_sync_state_2
%config %{conf_dir}/.drbd_sync_state_3
%config %{conf_dir}/.drbd_sync_state_4
%config %{conf_dir}/nodeinit.conf
%config %{conf_dir}/saflog.conf

########################################
# Variable Data files & Startup Scripts 
########################################

%defattr(755,root,root)
%{scripts_dir}/tipc_reset.sh
#%{scripts_dir}/node_id
%{scripts_dir}/pssv_spcn_list

%{scripts_dir}/node_ha_state
%{scripts_dir}/get_ha_state
%{scripts_dir}/drbd_clean.sh
%{scripts_dir}/drbd_start.sh
%{scripts_dir}/fabric_bond_conf
%{scripts_dir}/find_pid.sh
%{scripts_dir}/ncs_scap_clean.sh
%{scripts_dir}/ncs_scap_start.sh
%{scripts_dir}/ncs_cpd_clean.sh
%{scripts_dir}/ncs_cpd_start.sh
%{scripts_dir}/ncs_cpnd_clean.sh
%{scripts_dir}/ncs_cpnd_start.sh
%{scripts_dir}/ncs_dts_cleanup.sh
%{scripts_dir}/ncs_dts_start.sh
%{scripts_dir}/ncs_eds_clean.sh
%{scripts_dir}/ncs_eds_start.sh
%{scripts_dir}/ncs_gld_clean.sh
%{scripts_dir}/ncs_gld_start.sh
%{scripts_dir}/ncs_glnd_clean.sh
%{scripts_dir}/ncs_glnd_start.sh
#%{scripts_dir}/ncs_hisv_clean.sh 
#%{scripts_dir}/ncs_hisv_start.sh 
%{scripts_dir}/ncs_ifd_clean.sh
%{scripts_dir}/ncs_ifd_start.sh
%{scripts_dir}/ncs_ifnd_clean.sh
%{scripts_dir}/ncs_ifnd_start.sh
%{scripts_dir}/ncs_mas_cleanup.sh
%{scripts_dir}/ncs_mas_comp_name.txt
%{scripts_dir}/ncs_mas_start.sh
%{scripts_dir}/ncs_mqd_clean.sh
%{scripts_dir}/ncs_mqd_start.sh
%{scripts_dir}/ncs_mqnd_clean.sh
%{scripts_dir}/ncs_mqnd_start.sh
%{scripts_dir}/ncs_pss_cleanup.sh
%{scripts_dir}/ncs_pss_comp_name.txt
%{scripts_dir}/ncs_pss_start.sh
%{scripts_dir}/ncs_rde_clean.sh
%{scripts_dir}/ncs_rde_start.sh
%{scripts_dir}/ncs_snmp_subagt_reload.sh
%{scripts_dir}/ncs_snmpsubagt_cleanup.sh
%{scripts_dir}/ncs_snmpsubagt_start.sh
%{scripts_dir}/ncs_srmnd_clean.sh
%{scripts_dir}/ncs_srmnd_start.sh
%{scripts_dir}/ncs_vds_clean.sh
%{scripts_dir}/ncs_vds_start.sh
%{scripts_dir}/nid_tipc.sh
%{scripts_dir}/nis_scxb
%{scripts_dir}/nis_snmpd_start.sh
%{scripts_dir}/nis_snmpd_clean.sh
%{scripts_dir}/openhpid_clean.sh
%{scripts_dir}/openhpid_start.sh
%{scripts_dir}/openhpisubagt_clean.sh
%{scripts_dir}/openhpisubagt_start.sh
%{scripts_dir}/opensaf_lgs.sh
%{scripts_dir}/rde_script
%{scripts_dir}/drbd.1.conf
%{scripts_dir}/drbd.2.conf
%{scripts_dir}/pdrbd_proxied.conf
%{scripts_dir}/pdrbdsts
%{scripts_dir}/pdrbdctrl
%{scripts_dir}/drbdctrl
%{scripts_dir}/ncs_drbd_r0_clean.sh
%{scripts_dir}/ip_inst_script
%{scripts_dir}/collect_logs.sh
