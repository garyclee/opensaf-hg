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
# Author(s): Wind River Systems
#

AC_PREREQ(2.59)
AC_INIT(opensaf, 3.0.B2, users@list.opensaf.org)
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([OpenSAFCopyRights])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(opensaf, 3.0.B2)

# (major version) = (first number) - (third number)
# (minor version) = (third number)
# (patch version) = (second number)
OPENSAF_LIB_VERSION=3:0:0
AC_SUBST(OPENSAF_LIB_VERSION)

OPENSAF_RELEASE=$PACKAGE_VERSION
OPENSAF_BUGREPORT=$PACKAGE_BUGREPORT
AC_SUBST(OPENSAF_RELEASE)
AC_SUBST(OPENSAF_BUGREPORT)
AC_SUBST(ac_configure_args)


AC_ARG_WITH([rpm-release],
   [AS_HELP_STRING([--with-rpm-release=@<:@ARG@:>@],
      [Set the RPM release value to be current timestamp (ARG=timestamp);
      Set the RPM release value to be the global revision (ARG=global-rev);
      Set the RPM release value to be the local revision (ARG=local-rev);
      Set the RPM release value to be <val> (ARG=<val>, [default=1])])],
   [ if test "$withval" = timestamp; then
       with_rpm_release=`date +%Y%m%d%k%M`
     elif test "$withval" = global-rev; then
       with_rpm_release=`hg id -i`
     elif test "$withval" = local-rev; then
       with_rpm_release=`hg id -n`
     else
       with_rpm_release=$withval
     fi
   ],
   [with_rpm_release=1])

AC_SUBST([OPENSAF_RPM_RELEASE], ["$with_rpm_release"])


dnl Flag to enable smidump to be built
AC_ARG_ENABLE([smidump],
   [AS_HELP_STRING([--disable-smidump],
     [disable building the custom NCS smidump tool [default=no]])],
   [],
   [enable_smidump=yes])

AM_CONDITIONAL([ENABLE_SMIDUMP], [test "x$enable_smidump" = xyes])
AC_SUBST([SMIDUMP_ENABLED], ["$enable_smidump"])


dnl Enable HPI hardware management support
AC_ARG_ENABLE([hpi],
   [AS_HELP_STRING([--enable-hpi],
      [enable hardware management with HPI [default=yes]])],
   [],
   [enable_hpi=yes])

AM_CONDITIONAL([ENABLE_HPI], [test "x$enable_hpi" = xyes])
AC_SUBST([HPI_ENABLED], ["$enable_hpi"])


dnl Enable OpenHPI as the HPI implementation used
AC_ARG_WITH([openhpi],
   [AS_HELP_STRING([--with-openhpi],
      [enable OpenHPI as the HPI implementation [default=yes]])],
   [ if test "x$enable_hpi" = xyes; then
       with_openhpi="$withval"
     fi
   ],
   [ if test "x$enable_hpi" = xyes; then
       with_openhpi=yes
     else
       with_openhpi=no
     fi
   ])

AM_CONDITIONAL([WITH_OPENHPI], [test "x$with_openhpi" = xyes])
AC_SUBST([OPENHPI_ENABLED], ["$with_openhpi"])


dnl Specifies the SAI-HPI specification used by the HPI implementation
AC_ARG_WITH([hpirev],
   [AS_HELP_STRING([--with-hpirev=@<:@ARG@:>@],
      [HPI implementation is SAI-HPI-B.02.01 (ARG=B02 default); HPI implementation is
      SAI-HPI-B.01.01 (ARG=B01); HPI implementation is SAI-HPI-A.01.01
      (ARG=A01)])],
   [ if test "x$withval" = xB02; then
       with_hpirev=B02
       HPI_DEF="-DHPI_B_02"
     elif test "x$withval" = xB01; then
       with_hpirev=B01
       HPI_DEF="-DHPI_B_01"
     elif test "x$withval" = xA01; then
       with_hpirev=A01
       HPI_DEF="-DHPI_A"
     else
       echo
       echo "Unknown SAI-HPI specification version"
       exit -1
     fi
   ],
   [with_hpirev=B02])

AC_SUBST(HPI_DEF)

m4_include([m4/java.m4])

dnl Enable Java AIS interface mapping
AC_ARG_ENABLE([java],
   [AS_HELP_STRING([--enable-java],
      [enable the Java AIS interface mapping [default=no]])],
   [],
   [enable_java=no])

AM_CONDITIONAL([ENABLE_JAVA], [test "x$enable_java" = xyes])
AC_SUBST([JAVA_ENABLED], ["$enable_java"])

#############################################
# Get input for enabling platform interface
#############################################
AC_ARG_VAR(mot_atca, [Use mot_atca=1 if platform interface is needed])
AM_CONDITIONAL([MOT_ATCA], [test "$mot_atca" = "1"])

if test "$mot_atca" = "1"; then
    MOT_ATCA_VAL=1
    MOT_ATCA_STR="mot_atca"
    NETSNMP_5_2_2=1
else
    MOT_ATCA_VAL=0
    MOT_ATCA_STR=""
    NETSNMP_5_2_2=0
fi

AC_SUBST(MOT_ATCA_STR)

# unless explicitly set, drop previx from sysconfdir
# and localstatedir

if test "$sysconfdir" = '${prefix}/etc' ; then
  if test "$prefix" = 'NONE' ; then
    sysconfdir="/etc"
  else
    sysconfdir="$prefix"/etc
  fi
fi

AC_SUBST(sysconfdir)

if test "$localstatedir" = '${prefix}/var' ; then
  if test "$prefix" = 'NONE' ; then
    localstatedir="/var"
  else
    localstatedir="$prefix"/var
  fi
fi

AC_SUBST(localstatedir)

#############################################
# Get input for debug/nondebug build
#############################################
#FIXME: At present enable debug is hard coded.
DEBUG_FLAG="-DNCS_DEBUG=1"
DEBUG_VAL="1"
#AC_ARG_ENABLE(debug, [--enable-debug Enable Debug level for compilation],
#                [DEBUG_FLAG="-DNCS_DEBUG=1"
#		 DEBUG_VAL="1"],
#                [DEBUG_FLAG="-DNDEBUG"
#		 DEBUG_VAL="0"])

#############################################
# Define a common string for core CPP defines
#############################################
AC_SUBST( NCS_CORE_CPP_DEFINES, "-DNCS_NID=1 -DNCS_MTREE_PAT_STYLE_SUPPORTED=1 -DNCS_MMGR_DEBUG=$DEBUG_VAL -DENABLE_LEAP_DBG=$DEBUG_VAL -DNCSMDS_DEBUG=$DEBUG_VAL -DNCS_IPV6=0 -DNCS_IFSV_IPXS=1 -DNCS_IFSV_USE_NETLINK=1 -DNCS_TMR_DBG_ENABLE=$DEBUG_VAL -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 -DFUND_DEBUG_TEST=0 -DNCS_MONC_CPSV_USE=0 -DNCS_SAF=1 -DNCS_RMS=0 -DNCS_MAB=1 -DMOT_ATCA=$MOT_ATCA_VAL -DNET_SNMP_5_2_2_SUPPORT=$NETSNMP_5_2_2  -DNCS_MDS=1 -DNCSMDS_LOG=1 -DDTS_SIM_TEST_ENV=1 -D__JSE_LINUX__ -D__JSE_UNIX__ -DHJ_LINUX=1 -DUSE_READLINE -DDTA_FLOW=1 $DEBUG_FLAG")

#CFLAGS="$CFLAGS -fPIC -Wall -Wmissing-prototypes -Wno-pointer-sign -Werror"
#CXXFLAGS="$CXXFLAGS -fPIC -Wall -Wno-deprecated -Wno-write-strings -Werror"
AM_CFLAGS="-fPIC -Wall"
AM_CXXFLAGS="-fPIC -Wall"
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)

#############################################
# Set Cross Compile options if any
#############################################

AC_ARG_VAR(cc_exec_prefix, [Specify cross-compiler toolchain executables prefix Eg. For ppc_74xx-gcc specify cc_exec_prefix=ppc_74xx])
AC_ARG_VAR(cc_lib_dir, [Specify cross-compiled libraries path to be included with -L])
	
#########################################
# Exporting CC variables
#########################################
if ! test "$cc_exec_prefix" = ""; then
	export AR="$cc_exec_prefix-ar"
	export AS="$cc_exec_prefix-as"
	export CC="$cc_exec_prefix-gcc"
	export CPP="$cc_exec_prefix-cpp"
	export CXX="$cc_exec_prefix-g++"
	export CXXCPP="$cc_exec_prefix-g++ -E" 
	export LD="$cc_exec_prefix-ld"
	export RANLIB="$cc_exec_prefix-ranlib" 
fi
if ! test "$cc_lib_dir" = ""; then
	export LDFLAGS="$LDFLAGS -L$cc_lib_dir" 
fi

#############################################
# Checks for programs.
#############################################
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_YACC
AC_CHECK_TOOL(NETSNMP, net-snmp-config, MISSING)

if test "x$enable_java" = xyes; then
   AC_JAVA_WITH_JDK

   if test "$ac_java_jvm_dir" != ""; then
      AC_SUBST(JAVAC, $ac_java_jvm_dir"bin/javac")
      AC_SUBST(JAVADOC, $ac_java_jvm_dir"bin/javadoc")
   else
      AC_PROG_JAVAC
      AC_PROG_JAVADOC
   fi

   AC_JAVA_ANT
   AC_SUBST(JAVA_HOME, $ac_java_jvm_dir)
fi

#############################################
# MDS ARCH TYPE: MDS advises message encode/decode
# optimization to its clients if the sender and receiver
# are of "compatible-type". For this optimization to work, 
# the sender's platform and receiver's platform should be 
# similar (for e.g. both PowerPC) and use an libncs_core built 
# with the same value for "mds_arch". An "mds_arch" value of 0 
# is the default value and stands for a platform which is NOT
# compatible with any platform. 
#############################################

AC_ARG_VAR(mds_arch, [The arch-type input to MDS - valid values 1-7])
if test "$mds_arch" = ""; then
    mdsarchtype=0
else
    mdsarchtype=$mds_arch
fi
AC_SUBST( MDS_WORD_ARCH_FLAGS," -DMDS_ARCH_TYPE=$mdsarchtype" )

#############################################
# Checks for libraries.
#############################################
if test "x$enable_hpi" = xyes; then
   if test "x$with_openhpi" = xyes; then
      PKG_CHECK_MODULES(HPI, openhpi)
   fi
fi

# TODO: Fix the hardcoded locations, and add flexible --with
XERCESC_LIB=""
XERCESC_INCLUDE=""

AC_CHECK_LIB([xerces-c],
   [main],
   [XERCESC_LIB="-lxerces-c"],
   [ echo ""
     echo "*******************************************"
     echo "* Can't find the xerces shared libraries! *"
     echo "*******************************************"
     exit -1
   ])

AC_CHECK_HEADER([xercesc/util/XercesVersion.hpp],
   [XERCESC_INCLUDE="-I/usr/include/xercesc"],
   [ echo ""
     echo "**********************************************"
     echo "* Can't find the xerces development headers! *"
     echo "**********************************************"
     exit -1
   ])

AC_SUBST(XERCESC_LIB)
AC_SUBST(XERCESC_INCLUDE)

if test $NETSNMP = MISSING; then
	echo ""
	echo "***********************************************"
	echo "* Can't find the net-snmp development package *"
	echo "***********************************************"
	exit -1
else
   NETSNMP_LIBS=`net-snmp-config --agent-libs`
   NETSNMP_CFLAGS=`net-snmp-config --cflags`
   MIBSDIR=`net-snmp-config --default-mibdirs`
   AC_SUBST(NETSNMP_LIBS)
   AC_SUBST(NETSNMP_CFLAGS)
   AC_SUBST(MIBSDIR)
fi

#############################################
# Checks for header files.
#############################################
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h \
   stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h \
   sys/time.h syslog.h termios.h unistd.h])

#############################################
# Checks for typedefs, structures, and compiler characteristics.
#############################################
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_STRUCT_TM
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_SIGNAL

#############################################
# Checks for library functions.
#############################################
AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero clock_gettime dup2 ftruncate gethostname gettimeofday \
   inet_ntoa isascii localtime_r memmove memset mkfifo munmap pow select \
   setenv socket strcasecmp strchr strdup strerror strpbrk strrchr strspn \
   strstr strtol strtoul])

#############################################
# List the output Makefiles
#############################################
AC_CONFIG_FILES([opensaf.spec
                 Makefile
                 config/script.conf
                 scripts/collect_logs_controller
                 scripts/collect_logs_payload
                 scripts/dhclnt_clean
                 scripts/dhclnt_start
                 scripts/drbd_start
                 scripts/drbdctrl
                 scripts/get_ha_state
                 scripts/ip_inst_script
                 scripts/ncs_cpd_start
                 scripts/ncs_cpnd_start
                 scripts/ncs_dts_clean
                 scripts/ncs_dts_start
                 scripts/ncs_eds_clean
                 scripts/ncs_eds_start
                 scripts/ncs_gld_start
                 scripts/ncs_glnd_start
                 scripts/ncs_hisv_start
                 scripts/ncs_ifd_clean
                 scripts/ncs_ifd_start
                 scripts/ncs_ifnd_clean
                 scripts/ncs_ifnd_start
                 scripts/ncs_ifsv_ip_ins_script
                 scripts/ncs_mas_clean
                 scripts/ncs_mas_start
                 scripts/ncs_mqd_start
                 scripts/ncs_mqnd_start
                 scripts/ncs_pcap_clean
                 scripts/ncs_pcap_start
                 scripts/ncs_pss_clean
                 scripts/ncs_pss_start
                 scripts/ncs_rde_clean
                 scripts/ncs_rde_start
                 scripts/ncs_scap_clean
                 scripts/ncs_scap_start
                 scripts/ncs_snmpsubagt_clean
                 scripts/ncs_snmpsubagt_reload
                 scripts/ncs_snmpsubagt_start
                 scripts/ncs_srmnd_start
                 scripts/ncs_vds_start
                 scripts/nid_tipc
                 scripts/nis_snmpd_clean
                 scripts/nis_snmpd_start
                 scripts/openhpid_clean
                 scripts/openhpid_start
                 scripts/openhpisubagt_clean
                 scripts/openhpisubagt_start
                 scripts/opensaf_lgs
                 scripts/pdrbdctrl
                 scripts/rde_script
                 scripts/tipc_reset.sh
                 pkgconfig/opensaf-amf.pc
                 pkgconfig/opensaf-ckpt.pc
                 pkgconfig/opensaf-clm.pc
                 pkgconfig/opensaf-evt.pc
                 pkgconfig/opensaf-lck.pc
                 pkgconfig/opensaf-log.pc
                 pkgconfig/opensaf-msg.pc
                 java/Makefile
                 java/ais_api/Makefile
                 java/ais_api_impl/Makefile
                 java/ais_api_impl_native/Makefile
                 lib/lib_SaAmf/Makefile
                 lib/lib_SaCkpt/Makefile
                 lib/lib_SaMsg/Makefile
                 lib/lib_SaEvt/Makefile
                 lib/lib_SaClm/Makefile
                 lib/lib_SaLck/Makefile
                 lib/lib_SaLog/Makefile
                 lib/lib_ncs_core/Makefile
                 services/Makefile
                 services/avsv/Makefile
                 services/avsv/ava/Makefile
                 services/avsv/avd/Makefile
                 services/avsv/avm/Makefile
                 services/avsv/avm/snmp_integ/Makefile
                 services/avsv/avm/pssv_integ/Makefile
                 services/avsv/avm/logstr/Makefile
                 services/avsv/avnd/Makefile
                 services/avsv/cla/Makefile
                 services/avsv/clicmds/Makefile
                 services/avsv/common/Makefile
                 services/avsv/logstr/Makefile
                 services/avsv/pssv_integ/Makefile
                 services/avsv/snmp_integ/Makefile
                 services/bam/Makefile
                 services/bam/logstr/Makefile
                 services/cli/Makefile
                 services/cli/logstr/Makefile
                 services/common/Makefile
                 services/cpsv/Makefile
                 services/cpsv/common/Makefile
                 services/cpsv/cpa/Makefile
                 services/cpsv/cpd/Makefile
                 services/cpsv/cpnd/Makefile
                 services/cpsv/logstr/Makefile
                 services/cpsv/snmp_integ/Makefile
                 services/dtsv/Makefile
                 services/dtsv/clicmds/Makefile
                 services/dtsv/dta/Makefile
                 services/dtsv/dts/Makefile
                 services/dtsv/logstr/Makefile
                 services/dtsv/pssv_integ/Makefile
                 services/dtsv/snmp_integ/Makefile
                 services/edsv/Makefile
                 services/edsv/common/Makefile
                 services/edsv/eda/Makefile
                 services/edsv/eds/Makefile
                 services/edsv/logstr/Makefile
                 services/edsv/snmp_integ/Makefile
                 services/glsv/Makefile
                 services/glsv/common/Makefile
                 services/glsv/gla/Makefile
                 services/glsv/gld/Makefile
                 services/glsv/glnd/Makefile
                 services/glsv/logstr/Makefile
                 services/glsv/snmp_integ/Makefile
                 services/hisv/Makefile
                 services/hisv/hcd/Makefile
                 services/hisv/hpl/Makefile
                 services/hisv/logstr/Makefile
                 services/ifsv/Makefile
                 services/ifsv/clicmds/Makefile
                 services/ifsv/common/Makefile
                 services/ifsv/entp_ifsv_ir_snmp_integ/Makefile
                 services/ifsv/entp_snmp_integ/Makefile
                 services/ifsv/ifa/Makefile
                 services/ifsv/ifd/Makefile
                 services/ifsv/ifnd/Makefile
                 services/ifsv/ifsv_ip_installer/Makefile
                 services/ifsv/ipxs_snmp_integ/Makefile
                 services/ifsv/logstr/Makefile
                 services/ifsv/snmp_integ/Makefile
                 services/leap/Makefile
                 services/logsv/Makefile
                 services/logsv/lga/Makefile
                 services/logsv/lgs/Makefile
                 services/logsv/utils/Makefile
                 services/mab/Makefile
                 services/mab/logstr/Makefile
                 services/mab/mac/Makefile
                 services/mab/mas/Makefile
                 services/mab/oac/Makefile
                 services/mab/psr/Makefile
                 services/mab/psr/logstr/Makefile
                 services/mab/psr/snmp_integ/Makefile
                 services/mab/psrcli/Makefile
                 services/mbcsv/Makefile
                 services/mbcsv/logstr/Makefile
                 services/mbcsv/src/Makefile
                 services/mds/Makefile
                 services/mqsv/Makefile
                 services/mqsv/common/Makefile
                 services/mqsv/logstr/Makefile
                 services/mqsv/mqa/Makefile
                 services/mqsv/mqd/Makefile
                 services/mqsv/mqnd/Makefile
                 services/mqsv/snmp_integ/Makefile
                 services/nms/Makefile
                 services/nms/ncs-snmp-subagt/Makefile
                 services/nms/ncs-snmp-subagt/logstr/Makefile
                 services/nid/Makefile
                 services/nid/src/Makefile
                 services/srmsv/Makefile
                 services/srmsv/logstr/Makefile
                 services/srmsv/srma/Makefile
                 services/srmsv/srmnd/Makefile
                 services/rde/rda/Makefile
                 services/rde/Makefile
                 services/rde/logstr/Makefile
                 services/rde/src/Makefile
                 services/vds/Makefile
                 services/vds/logstr/Makefile
                 services/vds/vda/Makefile
                 services/vds/vds/Makefile
                 tools/hisv/Makefile
                 tools/pdrbd/Makefile
                 tools/pdrbd/logstr/Makefile
                 tools/pdrbd/src/Makefile])

AC_OUTPUT

echo ""
echo "======================================================="
echo "OpenSAF Build Configuration Summary"
echo "======================================================="
echo "Version: $VERSION"
echo "Release: $OPENSAF_RPM_RELEASE"
echo ""
echo "Installation Directories:"
echo "${ECHO_T} prefix: ${prefix}"
echo "${ECHO_T} bindir: ${bindir}"
echo "${ECHO_T} sbindir: ${sbindir}"
echo "${ECHO_T} datadir: ${datadir}"
echo "${ECHO_T} sysconfdir: ${sysconfdir}"
echo "${ECHO_T} docdir: ${docdir}"
echo "${ECHO_T} localstatedir: ${localstatedir}"
echo "${ECHO_T} includedir: ${includedir}"
echo "${ECHO_T} libdir: ${libdir}"
echo ""
echo "Compiling Options:"
echo "${ECHO_T} Target CPU: ${target_cpu}"
echo "${ECHO_T} C Compiler: ${CC}"
echo "${ECHO_T} C++ Compiler: ${CXX}"
echo "${ECHO_T} CFLAGS: ${CFLAGS} ${AM_CFLAGS}"
echo "${ECHO_T} CXXFLAGS: ${CXXFLAGS} ${AM_CXXFLAGS}"
echo "${ECHO_T} LDFLAGS: ${LDFLAGS}"
echo "${ECHO_T} srcdir: ${srcdir}"
if test "$srcdir" == "."; then
echo "${ECHO_T} Using VPATH: no"
else
echo "${ECHO_T} Using VPATH: yes"
fi
AM_CONDITIONAL([WITH_VPATH], [test "$srcdir" != "."])
echo ""
echo "Dependencies Options:"
echo "${ECHO_T} HPI Specification: ${with_hpirev}"
echo "${ECHO_T} HPI_CFLAGS: ${HPI_CFLAGS}"
echo "${ECHO_T} HPI_LIBS: ${HPI_LIBS}"
echo "${ECHO_T} NETSNMP_CFLAGS: ${NETSNMP_CFLAGS}"
echo "${ECHO_T} NETSNMP_LIBS: ${NETSNMP_LIBS}"
echo "${ECHO_T} MIBSDIR: ${MIBSDIR}"
echo ""
echo "OpenSAF Options:"
echo "${ECHO_T} Enable Java AIS Mapping: ${enable_java}"
echo "${ECHO_T} Enable HPI support: ${enable_hpi}"
echo "${ECHO_T} Enable OpenHPI: ${with_openhpi}"
echo "${ECHO_T} Enable Building NCS smidump: ${enable_smidump}"
echo "======================================================="
echo ""

