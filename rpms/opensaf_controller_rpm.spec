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

Summary: OpenSAF Services Controller Blade RPM
Name: opensaf_controller
Version:1.0 
Distribution: linux
Release:5
Source: %{name}.tgz
License: High-Availability Operating Environment Software License
Vendor: OpenSAF
Packager: user@opensaf.org
Group: System
AutoReqProv: 0
Requires: net-snmp=5.4, tipc, xerces=2.7.0
Conflicts: opensaf<=1.0.5
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
mkdir -p /etc/opt/opensaf/

#######################################
%post 
#######################################

mkdir -p /usr/share/snmp/
cp /etc/opt/opensaf/ncsSnmpSubagt.conf /usr/share/snmp/
mkdir -p /var/opt/opensaf/mdslog
mkdir -p /var/opt/opensaf/nidlog

########################################
####  Create soft links 
#######################################
mkdir -p /repl_opensaf/config
mkdir -p /repl_opensaf/pssv_store
mkdir -p /repl_opensaf/log

mv /opt/opensaf/controller/scripts/node_ha_state /var/opt/opensaf/
mv /etc/opt/opensaf/NCSSystemBOM.xml /repl_opensaf/config/NCSSystemBOM.xml
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/NCSSystemBOM.xml NCSSystemBOM.xml

mv /etc/opt/opensaf/ValidationConfig.xml   /repl_opensaf/config/ValidationConfig.xml
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/ValidationConfig.xml   ValidationConfig.xml

mv /etc/opt/opensaf/ValidationConfig.xsd   /repl_opensaf/config/ValidationConfig.xsd
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/ValidationConfig.xsd   ValidationConfig.xsd

mv /etc/opt/opensaf/AppConfig.xsd   /repl_opensaf/config/AppConfig.xsd
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/AppConfig.xsd   AppConfig.xsd

mv /etc/opt/opensaf/NCSConfig.xsd   /repl_opensaf/config/NCSConfig.xsd
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/NCSConfig.xsd   NCSConfig.xsd

mv /etc/opt/opensaf/vipsample.conf    /repl_opensaf/config/vipsample.conf 
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/vipsample.conf    vipsample.conf 

mv /etc/opt/opensaf/subagt_lib_conf   /repl_opensaf/config/subagt_lib_conf
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/subagt_lib_conf   subagt_lib_conf

mv /etc/opt/opensaf/cli_cefslib_conf    /repl_opensaf/config/cli_cefslib_conf 
cd /etc/opt/opensaf/
ln -s /repl_opensaf/config/cli_cefslib_conf    cli_cefslib_conf 

cd /opt/opensaf/
ln -s /opt/opensaf/controller/lib lib

cd /var/opt/opensaf 
ln -s /repl_opensaf/log  log   

cd /var/opt/opensaf
ln -s /repl_opensaf/pssv_store   pssv_store   

ln -sf /sbin/reboot /etc/opt/opensaf/reboot

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

echo "alias 'get_ha_state=/opt/opensaf/controller/scripts/get_ha_state'" >> /root/.bashrc
echo "alias 'nis_scxb=/opt/opensaf/controller/scripts/nis_scxb'" >> /root/.bashrc

source /root/.bashrc

########################################################

## Adding the LD_LIBRARY_PATH into /etc/ld.so.conf

check=`grep "/opt/opensaf/controller/lib/" /etc/ld.so.conf `
if [ "$check" == "" ]
then
   echo "/opt/opensaf/controller/lib/" >>/etc/ld.so.conf
fi

/sbin/ldconfig


#Uninstallation pre's
#######################################
%preun 
#######################################

###/etc/init.d/ncs_cb_initd stop 

#Uninstallation post: delet the softlink
#######################################
%postun
#######################################

userdel -rf opensaf
groupdel ncs_cli_superuser
groupdel ncs_cli_admin
groupdel ncs_cli_viewer

#Delete our files
rm -rf /opt/opensaf/controller/
rm -rf /opt/opensaf/include/
rm /opt/opensaf/lib
rm -rf /etc/opt/opensaf/
rm -rf /var/opt/opensaf/
rm -rf /repl_opensaf/

########################################
%files 
#######################################
###############################
#Ncs Services Controller Executables 
%defattr(755,root,root)
###############################
#/opt/opensaf/controller/
#/etc/opt/opensaf/
#/var/opt/opensaf/
#/repl_opensaf/
/opt/opensaf/controller/bin/ncs_cli_maa
/opt/opensaf/controller/bin/ncs_cpd
/opt/opensaf/controller/bin/ncs_cpnd
/opt/opensaf/controller/bin/ncs_dts
/opt/opensaf/controller/bin/ncs_eds
/opt/opensaf/controller/bin/ncs_gld
/opt/opensaf/controller/bin/ncs_glnd
#/opt/opensaf/controller/bin/ncs_hisv 
/opt/opensaf/controller/bin/ncs_ifd
/opt/opensaf/controller/bin/ncs_ifnd
/opt/opensaf/controller/bin/ncs_ifsv_ip_installer
/opt/opensaf/controller/bin/ncs_mas
/opt/opensaf/controller/bin/ncs_mqd
/opt/opensaf/controller/bin/ncs_mqnd
/opt/opensaf/controller/bin/ncs_nid
/opt/opensaf/controller/bin/ncs_psr
/opt/opensaf/controller/bin/ncs_rde
/opt/opensaf/controller/bin/ncs_scap
/opt/opensaf/controller/bin/ncs_snmp_subagt
/opt/opensaf/controller/bin/ncs_srmnd
/opt/opensaf/controller/bin/ncs_vds
/opt/opensaf/controller/bin/ncs_pdrbd


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

/opt/opensaf/controller/lib/libSaAmf.so
/opt/opensaf/controller/lib/libSaAmf.so.0
/opt/opensaf/controller/lib/libSaAmf.so.0.0.0
/opt/opensaf/controller/lib/libSaCkpt.so
/opt/opensaf/controller/lib/libSaCkpt.so.0
/opt/opensaf/controller/lib/libSaCkpt.so.0.0.0
/opt/opensaf/controller/lib/libSaClm.so
/opt/opensaf/controller/lib/libSaClm.so.0
/opt/opensaf/controller/lib/libSaClm.so.0.0.0
/opt/opensaf/controller/lib/libSaEvt.so
/opt/opensaf/controller/lib/libSaEvt.so.0
/opt/opensaf/controller/lib/libSaEvt.so.0.0.0
/opt/opensaf/controller/lib/libSaLck.so
/opt/opensaf/controller/lib/libSaLck.so.0
/opt/opensaf/controller/lib/libSaLck.so.0.0.0
/opt/opensaf/controller/lib/libSaMsg.so
/opt/opensaf/controller/lib/libSaMsg.so.0
/opt/opensaf/controller/lib/libSaMsg.so.0.0.0
/opt/opensaf/controller/lib/libavm_logstr.so
/opt/opensaf/controller/lib/libavm_logstr.so.1
/opt/opensaf/controller/lib/libavm_logstr.so.1.0.0
/opt/opensaf/controller/lib/libavm_pssv.so
/opt/opensaf/controller/lib/libavm_pssv.so.0
/opt/opensaf/controller/lib/libavm_pssv.so.0.0.0
/opt/opensaf/controller/lib/libavm_subagt.so
/opt/opensaf/controller/lib/libavm_subagt.so.0
/opt/opensaf/controller/lib/libavm_subagt.so.0.0.0
/opt/opensaf/controller/lib/libavsv_clicef.so
/opt/opensaf/controller/lib/libavsv_clicef.so.0
/opt/opensaf/controller/lib/libavsv_clicef.so.0.0.0
/opt/opensaf/controller/lib/libavsv_common.so
/opt/opensaf/controller/lib/libavsv_common.so.0
/opt/opensaf/controller/lib/libavsv_common.so.0.0.0
/opt/opensaf/controller/lib/libavsv_logstr.so
/opt/opensaf/controller/lib/libavsv_logstr.so.1
/opt/opensaf/controller/lib/libavsv_logstr.so.1.0.0
/opt/opensaf/controller/lib/libavsv_pssv.so
/opt/opensaf/controller/lib/libavsv_pssv.so.0
/opt/opensaf/controller/lib/libavsv_pssv.so.0.0.0
/opt/opensaf/controller/lib/libavsv_subagt.so
/opt/opensaf/controller/lib/libavsv_subagt.so.0
/opt/opensaf/controller/lib/libavsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libbam_logstr.so
/opt/opensaf/controller/lib/libbam_logstr.so.1
/opt/opensaf/controller/lib/libbam_logstr.so.1.0.0
/opt/opensaf/controller/lib/libcli_logstr.so
/opt/opensaf/controller/lib/libcli_logstr.so.1
/opt/opensaf/controller/lib/libcli_logstr.so.1.0.0
/opt/opensaf/controller/lib/libcpsv_common.so
/opt/opensaf/controller/lib/libcpsv_common.so.0
/opt/opensaf/controller/lib/libcpsv_common.so.0.0.0
/opt/opensaf/controller/lib/libcpsv_logstr.so
/opt/opensaf/controller/lib/libcpsv_logstr.so.1
/opt/opensaf/controller/lib/libcpsv_logstr.so.1.0.0
/opt/opensaf/controller/lib/libcpsv_subagt.so
/opt/opensaf/controller/lib/libcpsv_subagt.so.0
/opt/opensaf/controller/lib/libcpsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libdts_clicef.so
/opt/opensaf/controller/lib/libdts_clicef.so.0
/opt/opensaf/controller/lib/libdts_clicef.so.0.0.0
/opt/opensaf/controller/lib/libdts_logstr.so
/opt/opensaf/controller/lib/libdts_logstr.so.1
/opt/opensaf/controller/lib/libdts_logstr.so.1.0.0
/opt/opensaf/controller/lib/libdts_pssv.so
/opt/opensaf/controller/lib/libdts_pssv.so.0
/opt/opensaf/controller/lib/libdts_pssv.so.0.0.0
/opt/opensaf/controller/lib/libdtsv_subagt.so
/opt/opensaf/controller/lib/libdtsv_subagt.so.0
/opt/opensaf/controller/lib/libdtsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libedsv_common.so
/opt/opensaf/controller/lib/libedsv_common.so.0
/opt/opensaf/controller/lib/libedsv_common.so.0.0.0
/opt/opensaf/controller/lib/libedsv_logstr.so
/opt/opensaf/controller/lib/libedsv_logstr.so.1
/opt/opensaf/controller/lib/libedsv_logstr.so.1.0.0
/opt/opensaf/controller/lib/libedsv_subagt.so
/opt/opensaf/controller/lib/libedsv_subagt.so.0
/opt/opensaf/controller/lib/libedsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libglsv_common.so
/opt/opensaf/controller/lib/libglsv_common.so.0
/opt/opensaf/controller/lib/libglsv_common.so.0.0.0
/opt/opensaf/controller/lib/libglsv_logstr.so
/opt/opensaf/controller/lib/libglsv_logstr.so.2
/opt/opensaf/controller/lib/libglsv_logstr.so.2.0.0
/opt/opensaf/controller/lib/libglsv_subagt.so
/opt/opensaf/controller/lib/libglsv_subagt.so.0
/opt/opensaf/controller/lib/libglsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libhpl.so
/opt/opensaf/controller/lib/libhpl.so.0
/opt/opensaf/controller/lib/libhpl.so.0.0.0
#/opt/opensaf/controller/lib/libhisv_logstr.so
#/opt/opensaf/controller/lib/libhisv_logstr.so.1 
#/opt/opensaf/controller/lib/libhisv_logstr.so.1.0.0 
/opt/opensaf/controller/lib/libifa.so
/opt/opensaf/controller/lib/libifa.so.0
/opt/opensaf/controller/lib/libifa.so.0.0.0
/opt/opensaf/controller/lib/libifsv_bind_subagt.so
/opt/opensaf/controller/lib/libifsv_bind_subagt.so.0
/opt/opensaf/controller/lib/libifsv_bind_subagt.so.0.0.0
/opt/opensaf/controller/lib/libifsv_clicef.so
/opt/opensaf/controller/lib/libifsv_clicef.so.0
/opt/opensaf/controller/lib/libifsv_clicef.so.0.0.0
/opt/opensaf/controller/lib/libifsv_common.so
/opt/opensaf/controller/lib/libifsv_common.so.0
/opt/opensaf/controller/lib/libifsv_common.so.0.0.0
/opt/opensaf/controller/lib/libifsv_entp_subagt.so
/opt/opensaf/controller/lib/libifsv_entp_subagt.so.0
/opt/opensaf/controller/lib/libifsv_entp_subagt.so.0.0.0
/opt/opensaf/controller/lib/libifsv_logstr.so
/opt/opensaf/controller/lib/libifsv_logstr.so.1
/opt/opensaf/controller/lib/libifsv_logstr.so.1.0.0
/opt/opensaf/controller/lib/libifsv_subagt.so
/opt/opensaf/controller/lib/libifsv_subagt.so.0
/opt/opensaf/controller/lib/libifsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libipxs_subagt.so
/opt/opensaf/controller/lib/libipxs_subagt.so.0
/opt/opensaf/controller/lib/libipxs_subagt.so.0.0.0
#/opt/opensaf/controller/lib/liblfm_avm_intf.so 
#/opt/opensaf/controller/lib/liblfm_avm_intf.so.0 
#/opt/opensaf/controller/lib/liblfm_avm_intf.so.0.0.0 
/opt/opensaf/controller/lib/libmaa.so
/opt/opensaf/controller/lib/libmaa.so.0
/opt/opensaf/controller/lib/libmaa.so.0.0.0
/opt/opensaf/controller/lib/libmab_logstr.so
/opt/opensaf/controller/lib/libmab_logstr.so.2
/opt/opensaf/controller/lib/libmab_logstr.so.2.0.0
/opt/opensaf/controller/lib/libmbca.so
/opt/opensaf/controller/lib/libmbca.so.0
/opt/opensaf/controller/lib/libmbca.so.0.0.0
/opt/opensaf/controller/lib/libmbcsv_logstr.so
/opt/opensaf/controller/lib/libmbcsv_logstr.so.1
/opt/opensaf/controller/lib/libmbcsv_logstr.so.1.0.0
/opt/opensaf/controller/lib/libmqsv_common.so
/opt/opensaf/controller/lib/libmqsv_common.so.0
/opt/opensaf/controller/lib/libmqsv_common.so.0.0.0
/opt/opensaf/controller/lib/libmqsv_logstr.so
/opt/opensaf/controller/lib/libmqsv_logstr.so.2
/opt/opensaf/controller/lib/libmqsv_logstr.so.2.0.0
/opt/opensaf/controller/lib/libmqsv_subagt.so
/opt/opensaf/controller/lib/libmqsv_subagt.so.0
/opt/opensaf/controller/lib/libmqsv_subagt.so.0.0.0
/opt/opensaf/controller/lib/libncs_core.so
/opt/opensaf/controller/lib/libncs_core.so.0
/opt/opensaf/controller/lib/libncs_core.so.0.0.0
/opt/opensaf/controller/lib/libncs_snmp_subagt.so
/opt/opensaf/controller/lib/libncs_snmp_subagt.so.0
/opt/opensaf/controller/lib/libncs_snmp_subagt.so.0.0.0
/opt/opensaf/controller/lib/libpsr_clicef.so
/opt/opensaf/controller/lib/libpsr_clicef.so.0
/opt/opensaf/controller/lib/libpsr_clicef.so.0.0.0
/opt/opensaf/controller/lib/libpssv_logstr.so
/opt/opensaf/controller/lib/libpssv_logstr.so.3
/opt/opensaf/controller/lib/libpssv_logstr.so.3.0.0
/opt/opensaf/controller/lib/libpssv_subagt.so
/opt/opensaf/controller/lib/libpssv_subagt.so.0
/opt/opensaf/controller/lib/libpssv_subagt.so.0.0.0
/opt/opensaf/controller/lib/librda.so
/opt/opensaf/controller/lib/librda.so.0
/opt/opensaf/controller/lib/librda.so.0.0.0
/opt/opensaf/controller/lib/librde_logstr.so
/opt/opensaf/controller/lib/librde_logstr.so.1
/opt/opensaf/controller/lib/librde_logstr.so.1.0.0
/opt/opensaf/controller/lib/libsaf_common.so
/opt/opensaf/controller/lib/libsaf_common.so.0
/opt/opensaf/controller/lib/libsaf_common.so.0.0.0
/opt/opensaf/controller/lib/libsrma.so
/opt/opensaf/controller/lib/libsrma.so.0
/opt/opensaf/controller/lib/libsrma.so.0.0.0
/opt/opensaf/controller/lib/libsrmsv_logstr.so
/opt/opensaf/controller/lib/libsrmsv_logstr.so.2
/opt/opensaf/controller/lib/libsrmsv_logstr.so.2.0.0
/opt/opensaf/controller/lib/libsubagt_logstr.so
/opt/opensaf/controller/lib/libsubagt_logstr.so.1
/opt/opensaf/controller/lib/libsubagt_logstr.so.1.0.0
/opt/opensaf/controller/lib/libvds_logstr.so
/opt/opensaf/controller/lib/libvds_logstr.so.2
/opt/opensaf/controller/lib/libvds_logstr.so.2.0.0
/opt/opensaf/controller/lib/libpdrbd_logstr.so
/opt/opensaf/controller/lib/libpdrbd_logstr.so.1
/opt/opensaf/controller/lib/libpdrbd_logstr.so.1.0.0

###############################
#Ncs Services related SCXB conf file
###############################
%defattr(755,root,root)
 
# Startup scripts
###############################
%config /etc/opt/opensaf/NCSConfig.xsd
%config /etc/opt/opensaf/AppConfig.xsd
%config /etc/opt/opensaf/NCSSystemBOM.xml
%config /etc/opt/opensaf/ValidationConfig.xml
%config /etc/opt/opensaf/ValidationConfig.xsd
%config /etc/opt/opensaf/chassis_id
%config /etc/opt/opensaf/cli_cefslib_conf
%config /etc/opt/opensaf/ncsSnmpSubagt.conf
%config /etc/opt/opensaf/node_id
%config /etc/opt/opensaf/pssv_lib_conf
%config /etc/opt/opensaf/pssv_spcn_list
%config /etc/opt/opensaf/rde_rde_config_file
%config /etc/opt/opensaf/slot_id
%config /etc/opt/opensaf/subagt_lib_conf
%config /etc/opt/opensaf/tipc_reset.sh
%config /etc/opt/opensaf/vipsample.conf
%config /etc/opt/opensaf/.drbd_sync_state_0
%config /etc/opt/opensaf/.drbd_sync_state_1
%config /etc/opt/opensaf/.drbd_sync_state_2
%config /etc/opt/opensaf/.drbd_sync_state_3
%config /etc/opt/opensaf/.drbd_sync_state_4
%config /etc/opt/opensaf/nodeinit.conf

/opt/opensaf/controller/scripts/node_ha_state
/opt/opensaf/controller/scripts/get_ha_state
/opt/opensaf/controller/scripts/drbd_clean.sh
/opt/opensaf/controller/scripts/drbd_start.sh
/opt/opensaf/controller/scripts/fabric_bond_conf
/opt/opensaf/controller/scripts/find_pid.sh
/opt/opensaf/controller/scripts/ncs_cpd_clean.sh
/opt/opensaf/controller/scripts/ncs_cpd_start.sh
/opt/opensaf/controller/scripts/ncs_cpnd_clean.sh
/opt/opensaf/controller/scripts/ncs_cpnd_start.sh
/opt/opensaf/controller/scripts/ncs_dts_cleanup.sh
/opt/opensaf/controller/scripts/ncs_dts_start.sh
/opt/opensaf/controller/scripts/ncs_eds_clean.sh
/opt/opensaf/controller/scripts/ncs_eds_start.sh
/opt/opensaf/controller/scripts/ncs_gld_clean.sh
/opt/opensaf/controller/scripts/ncs_gld_start.sh
/opt/opensaf/controller/scripts/ncs_glnd_clean.sh
/opt/opensaf/controller/scripts/ncs_glnd_start.sh
#/opt/opensaf/controller/scripts/ncs_hisv_clean.sh 
#/opt/opensaf/controller/scripts/ncs_hisv_start.sh 
/opt/opensaf/controller/scripts/ncs_ifd_clean.sh
/opt/opensaf/controller/scripts/ncs_ifd_start.sh
/opt/opensaf/controller/scripts/ncs_ifnd_clean.sh
/opt/opensaf/controller/scripts/ncs_ifnd_start.sh
/opt/opensaf/controller/scripts/ncs_mas_cleanup.sh
/opt/opensaf/controller/scripts/ncs_mas_comp_name.txt
/opt/opensaf/controller/scripts/ncs_mas_start.sh
/opt/opensaf/controller/scripts/ncs_mqd_clean.sh
/opt/opensaf/controller/scripts/ncs_mqd_start.sh
/opt/opensaf/controller/scripts/ncs_mqnd_clean.sh
/opt/opensaf/controller/scripts/ncs_mqnd_start.sh
/opt/opensaf/controller/scripts/ncs_pss_cleanup.sh
/opt/opensaf/controller/scripts/ncs_pss_comp_name.txt
/opt/opensaf/controller/scripts/ncs_pss_start.sh
/opt/opensaf/controller/scripts/ncs_rde_clean.sh
/opt/opensaf/controller/scripts/ncs_rde_start.sh
/opt/opensaf/controller/scripts/ncs_snmp_subagt_reload.sh
/opt/opensaf/controller/scripts/ncs_snmpsubagt_cleanup.sh
/opt/opensaf/controller/scripts/ncs_snmpsubagt_start.sh
/opt/opensaf/controller/scripts/ncs_srmnd_clean.sh
/opt/opensaf/controller/scripts/ncs_srmnd_start.sh
/opt/opensaf/controller/scripts/ncs_vds_clean.sh
/opt/opensaf/controller/scripts/ncs_vds_start.sh
/opt/opensaf/controller/scripts/nid_tipc.sh
/opt/opensaf/controller/scripts/nis_scxb
/opt/opensaf/controller/scripts/nis_snmpd_start.sh
/opt/opensaf/controller/scripts/nis_snmpd_clean.sh
/opt/opensaf/controller/scripts/openhpid_clean.sh
/opt/opensaf/controller/scripts/openhpid_start.sh
/opt/opensaf/controller/scripts/openhpisubagt_clean.sh
/opt/opensaf/controller/scripts/openhpisubagt_start.sh
/opt/opensaf/controller/scripts/rde_script
/opt/opensaf/controller/scripts/drbd.1.conf
/opt/opensaf/controller/scripts/drbd.2.conf
/opt/opensaf/controller/scripts/pdrbd_proxied.conf
/opt/opensaf/controller/scripts/pdrbdsts
/opt/opensaf/controller/scripts/pdrbdctrl
/opt/opensaf/controller/scripts/drbdctrl
/opt/opensaf/controller/scripts/ncs_drbd_r0_clean.sh
/opt/opensaf/controller/scripts/ncs_ifsv_ip_ins_script
/opt/opensaf/controller/scripts/collect_logs.sh
