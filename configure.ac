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

AC_PREREQ([2.59])
AC_INIT([opensaf], [4.0.M3], [devel@list.opensaf.org])
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([OpenSAFCopyRights])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([$PACKAGE_NAME], [$PACKAGE_VERSION])

# (major version) = (first number) - (third number)
# (minor version) = (third number)
# (patch version) = (second number)
OPENSAF_LIB_VERSION=0:0:0
AC_SUBST([OPENSAF_LIB_VERSION])

OPENSAF_RELEASE=$PACKAGE_VERSION
OPENSAF_BUGREPORT=$PACKAGE_BUGREPORT
AC_SUBST([OPENSAF_RELEASE])
AC_SUBST([OPENSAF_BUGREPORT])
AC_SUBST([ac_configure_args])


#
# Check if rpath is disabled
#
AC_MSG_CHECKING([whether to use rpath])
AC_ARG_ENABLE([rpath],
	[AC_HELP_STRING([--disable-rpath],
		[Patches libtool to not use rpath in the libraries produced.])],
	[],
	[osaf_cv_rpath=yes])
AC_MSG_RESULT([$osaf_cv_rpath])

# Patch libtool to not use rpath if requested.
if test "$osaf_cv_rpath" = "no"; then
	AC_MSG_RESULT([Patching libtool to not use rpath.])
	sed < libtool > libtool-2 \
		's/^hardcode_libdir_flag_spec.*$'/'hardcode_libdir_flag_spec=" -D__LIBTOOL_NO_RPATH__ "/'
	mv libtool-2 libtool
	chmod 755 libtool
fi


#
# The RPM spec file has a container for the package release number (default to
# one), it is possible to customize that value with this configure option.
#
AC_ARG_WITH([rpm-release],
        [AS_HELP_STRING([--with-rpm-release=@<:@ARG@:>@],
                [set the RPM release value to be current timestamp (ARG=timestamp);
                set the RPM release value to be the global revision (ARG=global-rev);
                set the RPM release value to be the local revision (ARG=local-rev);
                set the RPM release value to be <val> (ARG=<val>, [default=1])])],
        [
        if test "$withval" = timestamp; then
                with_rpm_release=`date +%Y%m%d%k%M`
        elif test "$withval" = global-rev; then
                with_rpm_release=`hg id -i`
        elif test "$withval" = local-rev; then
                with_rpm_release=c`hg id -n`
        else
                with_rpm_release=$withval
        fi
        ],
        [with_rpm_release=1])

AC_SUBST([OPENSAF_RPM_RELEASE], ["$with_rpm_release"])


#
# OpenSAF supports linking with various SAHPI interface version, by default it
# will autodetect the version by looking at the SaHpi.h used at compilation,
# when cross-compiling this is not possible, it's possible to provide the
# version manually
#
AC_ARG_WITH([hpi-interface],
        [AS_HELP_STRING([--with-hpi-interface=@<:@ARG@:>@],
                [autodetect the SAHPI_INTERFACE_VERSION (ARG=check [default]);
                set the SAHPI_INTERFACE_VERSION to A.01.01 (ARG=A01);
                set the SAHPI_INTERFACE_VERSION to B.01.01 (ARG=B01);
                set the SAHPI_INTERFACE_VERSION to B.02.01 (ARG=B02);
                set the SAHPI_INTERFACE_VERSION to B.03.01 (ARG=B03)])],
        [
        if test "$withval" = A01; then
                with_hpi_interface=check
        elif test "$withval" = A01; then
                with_hpi_interface=A01
                AC_DEFINE([HAVE_HPI_A01], 1)
        elif test "$withval" = B01; then
                with_hpi_interface=B01
                AC_DEFINE([HAVE_HPI_B01], 1)
        elif test "$withval" = B02; then
                with_hpi_interface=B02
                AC_DEFINE([HAVE_HPI_B02], 1)
        elif test "$withval" = B03; then
                with_hpi_interface=B03
                AC_DEFINE([HAVE_HPI_B03], 1)
        else
                AC_ERROR([Invalid SAHPI_INTERFACE_VERSION specified])
        fi
        ],
        [with_hpi_interface=check])

AC_SUBST([WITH_HPI_INTERFACE], ["$with_hpi_interface"])


#
# Some build systems support their own 'make rpm' rule, offer the possibility to
# disable the one provided with OpenSAF
#
AC_MSG_CHECKING([whether to enable the make rpm target])
AC_ARG_ENABLE([rpm-target],
        [AS_HELP_STRING([--disable-rpm-target],
                [disable support for the "make rpm" target [default=no]])],
        [],
        [enable_rpm_target=yes])
AC_MSG_RESULT([$enable_rpm_target])

AM_CONDITIONAL([ENABLE_RPM_TARGET], [test "$enable_rpm_target" = yes])


#
# Option to enable/disable smidump to be built. It's possible to keep a local
# copy of the modified libsmi, in this case you don't need to build it each time
# when building OpenSAF.
#
AC_MSG_CHECKING([whether to enable building the custom smidump tool])
AC_ARG_ENABLE([smidump],
        [AS_HELP_STRING([--disable-smidump],
                [disable building the custom NCS smidump tool [default=no]])],
        [],
        [enable_smidump=yes])
AC_MSG_RESULT([$enable_smidump])

AM_CONDITIONAL([ENABLE_SMIDUMP], [test "$enable_smidump" = yes])
AC_SUBST([SMIDUMP_ENABLED], ["$enable_smidump"])


#
# Many distros use different init.d directory depending on their LSB compliance
# support. Use this option to point to the init.d directory used by your distro.
# Default to /etc/init.d
#
AC_ARG_WITH([initscriptdir],
        [AS_HELP_STRING([--with-initscriptdir=@<:@DIR@:>@],
                [use DIR as the directory containing the init.d scripts.
		 [default=/etc/init.d]])],
        [
        if test -d $withval; then
                with_initscriptdir=$withval
        else
                AC_ERROR([$withval is an invalid path])
        fi
        ],
        [with_initscriptdir=/etc/init.d])

AC_SUBST([initscriptdir], ["$with_initscriptdir"])


#
# By default MIBDIRS rely on net-snmp-config to get the system value, some
# distro provide a broken value for 'net-snmp-config --default-mibdirs', if it's
# your case, force it manually.
#
AC_ARG_WITH([default-mibdirs],
        [AS_HELP_STRING([--with-default-mibdirs=@<:@DIR@:>@],
                [use DIR as the default location to check for system MIBS])],
        [
        if test -d $withval; then
                with_mibdirs=$withval
        else
                AC_ERROR([$withval is an invalid path])
        fi
        ],
        [with_mibdirs=check])


#
# When cross-compiling you might have issue detecting/running the target copy of
# net-snmp-config (because it relies on the arch command at runtime), if it's
# your case, force the location of the 'net-snmp-config-<arch>' to get the proper
# value for your target.
#
AC_ARG_WITH([net-snmp-config-bin],
        [AS_HELP_STRING([--with-net-snmp-config-bin=@<:@FILE@:>@],
                [use FILE as the target specific net-snmp-config script])],
        [
        if test -x $withval; then
                with_netsnmp_config_bin=$withval
        else
                AC_ERROR([$withval is an invalid filename])
        fi
        ],
        [with_netsnmp_config_bin=net-snmp-config])

AC_SUBST([NETSNMP_CONFIG_BIN], ["$with_netsnmp_config_bin"])


#
# When cross-compiling you might have issue detecting/running the target copy of
# xml2-config, if it's your case, force the location of the xml2-config script
# to get the proper value for your target.
#
AC_ARG_WITH([xml2-config-bin],
        [AS_HELP_STRING([--with-xml2-config-bin=@<:@FILE@:>@],
                [use FILE as the target specific xml2-config script])],
        [
        if test -x $withval; then
                with_xml2_config_bin=$withval
        else
                AC_ERROR([$withval is an invalid filename])
        fi
        ],
        [with_xml2_config_bin=xml2-config])

AC_SUBST([XML2_CONFIG_BIN], ["$with_xml2_config_bin"])


#
# Enable/disable global HPI hardware management support
#
AC_MSG_CHECKING([whether to enable the HPI integration support])
AC_ARG_ENABLE([hpi],
        [AS_HELP_STRING([--enable-hpi],
                [enable hardware management with HPI [default=yes]])],
        [],
        [enable_hpi=yes])
AC_MSG_RESULT([$enable_hpi])

AM_CONDITIONAL([ENABLE_HPI], [test "$enable_hpi" = yes])
AC_SUBST([HPI_ENABLED], ["$enable_hpi"])


#
# Enable OpenHPI as the HPI implementation to be used for linking
#
AC_ARG_WITH([openhpi],
        [AS_HELP_STRING([--with-openhpi],
                [enable OpenHPI as the HPI implementation [default=yes]])],
        [
        if test "$enable_hpi" = yes; then
                with_openhpi="$withval"
        fi
        ],
        [
        if test "$enable_hpi" = yes; then
                with_openhpi=yes
        else
                with_openhpi=no
        fi
        ])

AM_CONDITIONAL([WITH_OPENHPI], [test "$with_openhpi" = yes])
AC_SUBST([OPENHPI_ENABLED], ["$with_openhpi"])


#
# Enable/disable the Java AIS interface mapping components
#
m4_include([m4/java.m4])

AC_MSG_CHECKING([whether to enable the Java AIS interface mapping])
AC_ARG_ENABLE([java],
        [AS_HELP_STRING([--enable-java],
                [enable the Java AIS interface mapping [default=no]])],
        [],
        [enable_java=no])
AC_MSG_RESULT([$enable_java])

AM_CONDITIONAL([ENABLE_JAVA], [test "$enable_java" = yes])
AC_SUBST([JAVA_ENABLED], ["$enable_java"])


#
# Enable/disable building the OpenSAF tests
#
AC_MSG_CHECKING([whether to enable building the OpenSAF tests])
AC_ARG_ENABLE([tests],
        [AS_HELP_STRING([--enable-tests],
                [enable building the OpenSAF testing programs [default=no]])],
        [],
        [enable_tests=no])
AC_MSG_RESULT([$enable_tests])

AM_CONDITIONAL([ENABLE_TESTS], [test "$enable_tests" = yes])
AC_SUBST([TESTS_ENABLED], ["$enable_tests"])


#
# unless explicitly set, drop previx from sysconfdir
# and localstatedir
#
if test "$sysconfdir" = '${prefix}/etc' ; then
        if test "$prefix" = 'NONE' ; then
                sysconfdir="/etc"
        else
                sysconfdir="$prefix"/etc
        fi
fi

AC_SUBST([sysconfdir])

if test "$localstatedir" = '${prefix}/var' ; then
        if test "$prefix" = 'NONE' ; then
                localstatedir="/var"
        else
                localstatedir="$prefix"/var
        fi
fi

AC_SUBST([localstatedir])


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

if test "$cross_compiling" = no; then
        AC_CHECK_PROG([ac_check_netsnmp_config], [$with_netsnmp_config_bin], 
                [found], [missing])
        AC_CHECK_PROG([ac_check_xml2_config], [$with_xml2_config_bin],
                [found], [missing])
fi

if test "$enable_java" = yes; then
        AC_JAVA_WITH_JDK

        if test "$ac_java_jvm_dir" != ""; then
                AC_SUBST([JAVAC], [$ac_java_jvm_dir"bin/javac"])
                AC_SUBST([JAVADOC], [$ac_java_jvm_dir"bin/javadoc"])
        else
                AC_PROG_JAVAC
                AC_PROG_JAVADOC
        fi

        AC_JAVA_ANT
        AC_SUBST([JAVA_HOME], [$ac_java_jvm_dir])
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

AC_ARG_VAR([mds_arch], [The arch-type input to MDS - valid values 1-7])

if test "$mds_arch" = ""; then
        mdsarchtype=0
else
        mdsarchtype=$mds_arch
fi

AC_SUBST([MDS_WORD_ARCH_FLAGS], [" -DMDS_ARCH_TYPE=$mdsarchtype"])

#############################################
# Checks for libraries.
#############################################
if test "$enable_hpi" = yes; then
   if test "$with_openhpi" = yes; then
      PKG_CHECK_MODULES([HPI], [openhpi])

      if test "$with_hpi_interface" = check; then
         if test "$cross_compiling" = no; then
            # Test for the SAHPI_INTERFACE_VERSION
            AC_RUN_IFELSE(
               AC_LANG_PROGRAM(
                  [#include <openhpi/SaHpi.h>],
                  [if (SAHPI_INTERFACE_VERSION != 0x010101) return 1;]
               ),
               AC_DEFINE(
                  [HAVE_HPI_A01], 1, [SAHPI_INTERFACE_VERSION == A01]
               )
            )
            AC_RUN_IFELSE(
               AC_LANG_PROGRAM(
                  [#include <openhpi/SaHpi.h>],
                  [if (SAHPI_INTERFACE_VERSION != 0x020101) return 1;]
               ),
               AC_DEFINE(
                  [HAVE_HPI_B01], 1, [SAHPI_INTERFACE_VERSION == B01]
               )
            )
            AC_RUN_IFELSE(
               AC_LANG_PROGRAM(
                  [#include <openhpi/SaHpi.h>],
                  [if (SAHPI_INTERFACE_VERSION != 0x020201) return 1;]
               ),
               AC_DEFINE(
                  [HAVE_HPI_B02], 1, [SAHPI_INTERFACE_VERSION == B02]
               )
            )
            AC_RUN_IFELSE(
               AC_LANG_PROGRAM(
                  [#include <openhpi/SaHpi.h>],
                  [if (SAHPI_INTERFACE_VERSION != 0x020301) return 1;]
               ),
               AC_DEFINE(
                  [HAVE_HPI_B03], 1, [SAHPI_INTERFACE_VERSION == B03]
               )
            )
         else
           AC_ERROR([Autodecting HPI interface version isn't supported while
           cross-compiling, force --with-hpi-version])
         fi
      fi
   fi
else
   # FIXME
   # Not supposed to be using HPI if here, but we currently
   # need a workaround because typedefs from SaHpi.h are used
   # even if HPI support is disabled in OpenSAF.

   if test "$with_hpi_interface" = check; then
      if test "$cross_compiling" = no; then
         # Test for the SAHPI_INTERFACE_VERSION
         AC_RUN_IFELSE(
            AC_LANG_PROGRAM(
               [#include <SaHpi.h>],
               [if (SAHPI_INTERFACE_VERSION != 0x010101) return 1;]
            ),
            AC_DEFINE(
               [HAVE_HPI_A01], 1, [SAHPI_INTERFACE_VERSION == A01]
            )
         )
         AC_RUN_IFELSE(
            AC_LANG_PROGRAM(
               [#include <SaHpi.h>],
               [if (SAHPI_INTERFACE_VERSION != 0x020101) return 1;]
            ),
            AC_DEFINE(
               [HAVE_HPI_B01], 1, [SAHPI_INTERFACE_VERSION == B01]
            )
         )
         AC_RUN_IFELSE(
            AC_LANG_PROGRAM(
               [#include <SaHpi.h>],
               [if (SAHPI_INTERFACE_VERSION != 0x020201) return 1;]
            ),
            AC_DEFINE(
               [HAVE_HPI_B02], 1, [SAHPI_INTERFACE_VERSION == B02]
            )
         )
         AC_RUN_IFELSE(
            AC_LANG_PROGRAM(
               [#include <SaHpi.h>],
               [if (SAHPI_INTERFACE_VERSION != 0x020301) return 1;]
            ),
            AC_DEFINE(
               [HAVE_HPI_B03], 1, [SAHPI_INTERFACE_VERSION == B03]
            )
         )
      else
         AC_ERROR([Autodecting HPI interface version isn't supported while
         cross-compiling, force --with-hpi-version])
      fi
   fi
fi

if test "$ac_check_netsnmp_config" = missing; then
        AC_ERROR([Can't find the net-snmp development package])
else
        NETSNMP_LIBS=`${NETSNMP_CONFIG_BIN} --agent-libs`
        NETSNMP_CFLAGS=`${NETSNMP_CONFIG_BIN} --cflags`
 
        if test "$with_mibdirs" = check; then
                MIBDIRS=`${NETSNMP_CONFIG_BIN} --default-mibdirs`
        else
                MIBDIRS=$with_mibdirs
        fi

        AC_SUBST([NETSNMP_LIBS])
        AC_SUBST([NETSNMP_CFLAGS])
        AC_SUBST([MIBDIRS])
fi

if test "$ac_check_xml2_config" = missing; then
        AC_ERROR([Can't find the libxml2 development package])
else
        XML2_LIBS=`${XML2_CONFIG_BIN} --libs`
        XML2_CFLAGS=`${XML2_CONFIG_BIN} --cflags`
        AC_SUBST([XML2_LIBS])
        AC_SUBST([XML2_CFLAGS])
fi


#############################################
# Checks for header files.
#############################################
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

AC_CHECK_HEADERS([ \
        arpa/inet.h \
        fcntl.h \
        limits.h \
        malloc.h \
        netdb.h \
        netinet/in.h \
        stddef.h \
        stdlib.h \
        string.h \
        sys/file.h \
        sys/ioctl.h \
        sys/param.h \
        sys/socket.h \
        sys/time.h \
        syslog.h \
        termios.h \
        unistd.h])


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

AC_CHECK_FUNCS([ \
        bzero \
        clock_gettime \
        dup2 \
        ftruncate \
        gethostname \
        gettimeofday \
        inet_ntoa \
        isascii \
        localtime_r \
        memmove \
        memset \
        mkfifo \
        munmap \
        pow \
        select \
        setenv \
        socket \
        strcasecmp \
        strchr \
        strdup \
        strerror \
        strpbrk \
        strrchr \
        strspn \
        strstr \
        strtol \
        strtoul])

#############################################
# List the output Makefiles
#############################################
AC_CONFIG_FILES([
        Makefile
        opensaf.spec
        Doxyfile
        config/script.conf
        java/Makefile
        java/ais_api/Makefile
        java/ais_api_impl/Makefile
        java/ais_api_impl_native/Makefile
        osaf/Makefile
        osaf/cli/Makefile
        osaf/cli/config/Makefile
        osaf/cli/logstr/Makefile
        osaf/cli/services/Makefile
        osaf/cli/services/avsv/Makefile
        osaf/cli/services/dtsv/Makefile
        osaf/cli/services/psr/Makefile
        osaf/snmp/Makefile
        osaf/snmp/config/Makefile
        osaf/snmp/mapi/Makefile
        osaf/snmp/mapi/avsv/Makefile
        osaf/snmp/mapi/avsv/avm/Makefile
        osaf/snmp/miblib/Makefile
        osaf/snmp/miblib/avsv/Makefile
        osaf/snmp/miblib/avsv/avm/Makefile
        osaf/snmp/miblib/dtsv/Makefile
        osaf/snmp/miblib/dtsv/dts/Makefile
        osaf/snmp/miblib/pssv/Makefile
        osaf/snmp/miblib/pssv/pss/Makefile
        osaf/snmp/mibs/Makefile
        osaf/snmp/pssv/Makefile
        osaf/snmp/pssv/config/Makefile
        osaf/snmp/pssv/avsv/Makefile
        osaf/snmp/pssv/avsv/avm/Makefile
        osaf/snmp/pssv/dtsv/Makefile
        osaf/snmp/scripts/Makefile
        osaf/snmp/scripts/ncs_snmpsubagt_clean
        osaf/snmp/scripts/ncs_snmpsubagt_reload
        osaf/snmp/scripts/ncs_snmpsubagt_start
        osaf/snmp/scripts/nis_snmpd_clean
        osaf/snmp/scripts/nis_snmpd_start
        osaf/snmp/subagt/Makefile
        osaf/snmp/subagt/config/Makefile
        osaf/snmp/subagt/logstr/Makefile
        osaf/snmp/subagt/services/Makefile
        osaf/snmp/subagt/services/avsv/Makefile
        osaf/snmp/subagt/services/avsv/avm/Makefile
        osaf/snmp/subagt/services/dtsv/Makefile
        osaf/snmp/subagt/services/pssv/Makefile
        osaf/libs/Makefile
        osaf/libs/agents/Makefile
        osaf/libs/agents/infrastructure/Makefile
        osaf/libs/agents/infrastructure/dta/Makefile
        osaf/libs/agents/infrastructure/fma/logstr/Makefile
        osaf/libs/agents/infrastructure/fma/Makefile
        osaf/libs/agents/infrastructure/maa/Makefile
        osaf/libs/agents/infrastructure/oaa/Makefile
        osaf/libs/agents/infrastructure/rda/Makefile
        osaf/libs/agents/saf/Makefile
        osaf/libs/agents/saf/ava/Makefile
        osaf/libs/agents/saf/ava/include/Makefile
        osaf/libs/agents/saf/imma/Makefile
        osaf/libs/agents/saf/lga/Makefile
        osaf/libs/agents/saf/ntfa/Makefile
        osaf/libs/agents/saf/cla/Makefile
        osaf/libs/agents/saf/cla/include/Makefile
        osaf/libs/agents/saf/cpa/Makefile
        osaf/libs/agents/saf/eda/Makefile
        osaf/libs/agents/saf/gla/Makefile
        osaf/libs/agents/saf/mqa/Makefile
        osaf/libs/core/Makefile
        osaf/libs/core/include/Makefile
        osaf/libs/core/common/Makefile
        osaf/libs/core/common/include/Makefile
        osaf/libs/core/leap/Makefile
        osaf/libs/core/leap/include/Makefile
        osaf/libs/core/mbcsv/Makefile
        osaf/libs/core/mbcsv/include/Makefile
        osaf/libs/core/mbcsv/logstr/Makefile
        osaf/libs/core/mds/Makefile
        osaf/libs/core/mds/include/Makefile
        osaf/libs/common/Makefile
        osaf/libs/common/avsv/Makefile
        osaf/libs/common/avsv/include/Makefile
        osaf/libs/common/cpsv/Makefile
        osaf/libs/common/cpsv/include/Makefile
        osaf/libs/common/edsv/Makefile
        osaf/libs/common/edsv/include/Makefile
        osaf/libs/common/glsv/Makefile
        osaf/libs/common/glsv/include/Makefile
        osaf/libs/common/immsv/Makefile
        osaf/libs/common/immsv/include/Makefile
        osaf/libs/common/logsv/Makefile
        osaf/libs/common/logsv/include/Makefile
        osaf/libs/common/mqsv/Makefile
        osaf/libs/common/mqsv/include/Makefile
        osaf/libs/common/ntfsv/Makefile
        osaf/libs/common/ntfsv/include/Makefile
        osaf/libs/saf/Makefile
        osaf/libs/saf/include/Makefile
        osaf/libs/saf/libSaAmf/Makefile
        osaf/libs/saf/libSaAmf/pkgconfig/Makefile
        osaf/libs/saf/libSaAmf/pkgconfig/opensaf-amf.pc
        osaf/libs/saf/libSaImm/Makefile
        osaf/libs/saf/libSaImm/pkgconfig/Makefile
        osaf/libs/saf/libSaImm/pkgconfig/opensaf-imm.pc
        osaf/libs/saf/libSaLog/Makefile
        osaf/libs/saf/libSaLog/pkgconfig/Makefile
        osaf/libs/saf/libSaLog/pkgconfig/opensaf-log.pc
        osaf/libs/saf/libSaNtf/Makefile
        osaf/libs/saf/libSaNtf/pkgconfig/Makefile
        osaf/libs/saf/libSaNtf/pkgconfig/opensaf-ntf.pc
        osaf/libs/saf/libSaClm/Makefile
        osaf/libs/saf/libSaClm/pkgconfig/Makefile
        osaf/libs/saf/libSaClm/pkgconfig/opensaf-clm.pc
        osaf/libs/saf/libSaCkpt/Makefile
        osaf/libs/saf/libSaCkpt/pkgconfig/Makefile
        osaf/libs/saf/libSaCkpt/pkgconfig/opensaf-ckpt.pc
        osaf/libs/saf/libSaEvt/Makefile
        osaf/libs/saf/libSaEvt/pkgconfig/Makefile
        osaf/libs/saf/libSaEvt/pkgconfig/opensaf-evt.pc
        osaf/libs/saf/libSaLck/Makefile
        osaf/libs/saf/libSaLck/pkgconfig/Makefile
        osaf/libs/saf/libSaLck/pkgconfig/opensaf-lck.pc
        osaf/libs/saf/libSaMsg/Makefile
        osaf/libs/saf/libSaMsg/pkgconfig/Makefile
        osaf/libs/saf/libSaMsg/pkgconfig/opensaf-msg.pc
        osaf/services/Makefile
        osaf/services/infrastructure/Makefile
        osaf/services/infrastructure/dtsv/Makefile
        osaf/services/infrastructure/dtsv/include/Makefile
        osaf/services/infrastructure/dtsv/logstr/Makefile
        osaf/services/infrastructure/dtsv/dts/Makefile
        osaf/services/infrastructure/dtsv/dts/scripts/Makefile
        osaf/services/infrastructure/dtsv/dts/scripts/ncs_dts_clean
        osaf/services/infrastructure/dtsv/dts/scripts/ncs_dts_start
        osaf/services/infrastructure/fm/Makefile
        osaf/services/infrastructure/fm/include/Makefile
        osaf/services/infrastructure/fm/fms/Makefile
        osaf/services/infrastructure/fm/config/Makefile
        osaf/services/infrastructure/fm/fms/scripts/Makefile
        osaf/services/infrastructure/fm/fms/scripts/opensaf_fms
        osaf/services/infrastructure/hisv/Makefile
        osaf/services/infrastructure/hisv/hcd/Makefile
        osaf/services/infrastructure/hisv/hpl/Makefile
        osaf/services/infrastructure/hisv/include/Makefile
        osaf/services/infrastructure/hisv/logstr/Makefile
        osaf/services/infrastructure/hisv/scripts/Makefile
        osaf/services/infrastructure/hisv/scripts/ncs_hisv_start
        osaf/services/infrastructure/masv/Makefile
        osaf/services/infrastructure/masv/include/Makefile
        osaf/services/infrastructure/masv/logstr/Makefile
        osaf/services/infrastructure/masv/mas/Makefile
        osaf/services/infrastructure/masv/mas/scripts/Makefile
        osaf/services/infrastructure/masv/mas/scripts/ncs_mas_clean
        osaf/services/infrastructure/masv/mas/scripts/ncs_mas_start
        osaf/services/infrastructure/masv/pss/Makefile
        osaf/services/infrastructure/masv/pss/logstr/Makefile
        osaf/services/infrastructure/masv/pss/scripts/Makefile
        osaf/services/infrastructure/masv/pss/scripts/ncs_pss_clean
        osaf/services/infrastructure/masv/pss/scripts/ncs_pss_start
        osaf/services/infrastructure/nid/Makefile
        osaf/services/infrastructure/nid/config/Makefile
        osaf/services/infrastructure/nid/scripts/Makefile
        osaf/services/infrastructure/nid/scripts/nid_tipc
        osaf/services/infrastructure/nid/scripts/opensafd
        osaf/services/infrastructure/pdrbd/Makefile
        osaf/services/infrastructure/pdrbd/config/Makefile
        osaf/services/infrastructure/pdrbd/logstr/Makefile
        osaf/services/infrastructure/pdrbd/scripts/Makefile
        osaf/services/infrastructure/pdrbd/scripts/drbd_start
        osaf/services/infrastructure/pdrbd/scripts/drbdctrl
        osaf/services/infrastructure/pdrbd/scripts/pdrbdsts
        osaf/services/infrastructure/pdrbd/scripts/ncs_drbd_r0_clean
        osaf/services/infrastructure/pdrbd/scripts/pdrbdctrl
        osaf/services/infrastructure/rde/Makefile
        osaf/services/infrastructure/rde/config/Makefile
        osaf/services/infrastructure/rde/logstr/Makefile
        osaf/services/infrastructure/rde/include/Makefile
        osaf/services/infrastructure/rde/scripts/Makefile
        osaf/services/infrastructure/rde/scripts/ncs_rde_clean
        osaf/services/infrastructure/rde/scripts/ncs_rde_start
        osaf/services/infrastructure/rde/scripts/rde_script
        osaf/services/saf/Makefile
        osaf/services/saf/avsv/Makefile
        osaf/services/saf/avsv/config/Makefile
        osaf/services/saf/avsv/logstr/Makefile
        osaf/services/saf/avsv/avm/Makefile
        osaf/services/saf/avsv/avm/include/Makefile
        osaf/services/saf/avsv/avm/logstr/Makefile
        osaf/services/saf/avsv/avd/Makefile
        osaf/services/saf/avsv/avd/include/Makefile
        osaf/services/saf/avsv/avnd/Makefile
        osaf/services/saf/avsv/avnd/include/Makefile
        osaf/services/saf/avsv/pcap/Makefile
        osaf/services/saf/avsv/pcap/scripts/Makefile
        osaf/services/saf/avsv/pcap/scripts/opensaf_pcap_clean
        osaf/services/saf/avsv/pcap/scripts/opensaf_pcap_start
        osaf/services/saf/avsv/scap/Makefile
        osaf/services/saf/avsv/scap/scripts/Makefile
        osaf/services/saf/avsv/scap/scripts/opensaf_scap_clean
        osaf/services/saf/avsv/scap/scripts/opensaf_scap_start
        osaf/services/saf/immsv/Makefile
        osaf/services/saf/immsv/config/Makefile
        osaf/services/saf/immsv/immd/Makefile
        osaf/services/saf/immsv/immnd/Makefile
        osaf/services/saf/immsv/immd/scripts/Makefile
        osaf/services/saf/immsv/immd/scripts/immd_script
        osaf/services/saf/immsv/immnd/scripts/Makefile
        osaf/services/saf/immsv/immnd/scripts/immnd_script
        osaf/services/saf/logsv/Makefile
        osaf/services/saf/logsv/config/Makefile
        osaf/services/saf/logsv/lgs/Makefile
        osaf/services/saf/logsv/lgs/scripts/Makefile
        osaf/services/saf/logsv/lgs/scripts/opensaf_lgs
        osaf/services/saf/ntfsv/Makefile
        osaf/services/saf/ntfsv/config/Makefile
        osaf/services/saf/ntfsv/ntfs/Makefile
        osaf/services/saf/ntfsv/ntfs/scripts/Makefile
        osaf/services/saf/ntfsv/ntfs/scripts/opensaf_ntfs
        osaf/services/saf/cpsv/Makefile
        osaf/services/saf/cpsv/logstr/Makefile
        osaf/services/saf/cpsv/cpd/Makefile
        osaf/services/saf/cpsv/cpnd/Makefile
        osaf/services/saf/cpsv/cpd/scripts/Makefile
        osaf/services/saf/cpsv/cpd/scripts/ncs_cpd_start
        osaf/services/saf/cpsv/cpnd/scripts/Makefile
        osaf/services/saf/cpsv/cpnd/scripts/ncs_cpnd_start
        osaf/services/saf/edsv/Makefile
        osaf/services/saf/edsv/logstr/Makefile
        osaf/services/saf/edsv/eds/Makefile
        osaf/services/saf/edsv/eds/scripts/Makefile
        osaf/services/saf/edsv/eds/scripts/ncs_eds_clean
        osaf/services/saf/edsv/eds/scripts/ncs_eds_start
        osaf/services/saf/glsv/Makefile
        osaf/services/saf/glsv/logstr/Makefile
        osaf/services/saf/glsv/gld/Makefile
        osaf/services/saf/glsv/glnd/Makefile
        osaf/services/saf/glsv/gld/scripts/Makefile
        osaf/services/saf/glsv/gld/scripts/ncs_gld_start
        osaf/services/saf/glsv/glnd/scripts/Makefile
        osaf/services/saf/glsv/glnd/scripts/ncs_glnd_start
        osaf/services/saf/mqsv/Makefile
        osaf/services/saf/mqsv/logstr/Makefile
        osaf/services/saf/mqsv/mqd/Makefile
        osaf/services/saf/mqsv/mqnd/Makefile
        osaf/services/saf/mqsv/mqd/scripts/Makefile
        osaf/services/saf/mqsv/mqd/scripts/ncs_mqd_start
        osaf/services/saf/mqsv/mqnd/scripts/Makefile
        osaf/services/saf/mqsv/mqnd/scripts/ncs_mqnd_start
        osaf/tools/Makefile
        osaf/tools/hisv_events_demo/Makefile
        osaf/tools/hisv_power_set_util/Makefile
        osaf/tools/safimm/Makefile
        osaf/tools/safimm/immadm/Makefile
        osaf/tools/safimm/immcfg/Makefile
        osaf/tools/safimm/immdump/Makefile
        osaf/tools/safimm/immfind/Makefile
        osaf/tools/safimm/immlist/Makefile
        osaf/tools/safimm/immload/Makefile
        osaf/tools/safimm/include/Makefile
        osaf/tools/saflog/Makefile
        osaf/tools/saflog/include/Makefile
        osaf/tools/saflog/saflogger/Makefile
        osaf/tools/safntf/Makefile
        osaf/tools/safntf/include/Makefile
        osaf/tools/safntf/ntfsend/Makefile
        osaf/tools/safntf/ntfsubscribe/Makefile
        pkgconfig/Makefile
        pkgconfig/opensaf.pc
        scripts/collect_logs_controller
        scripts/collect_logs_payload
        scripts/get_ha_state
        scripts/openhpid_clean
        scripts/openhpid_start
        scripts/openhpisubagt_clean
        scripts/openhpisubagt_start
        scripts/tipc_reset.sh
        tests/avsv/amfoitest/Makefile
        tests/immsv/implementer/Makefile
        tests/immsv/management/Makefile
        tests/logsv/Makefile
        tests/ntfsv/Makefile])

AC_OUTPUT

echo ""
echo "======================================================="
echo "OpenSAF Build Configuration Summary"
echo "======================================================="
echo " Version: $VERSION"
echo " Release: $OPENSAF_RPM_RELEASE"
echo ""
echo " Installation Directories:"
echo "${ECHO_T}  prefix: ${prefix}"
echo "${ECHO_T}  exec_prefix: ${exec_prefix}"
echo "${ECHO_T}  bindir: ${bindir}"
echo "${ECHO_T}  sbindir: ${sbindir}"
echo "${ECHO_T}  datadir: ${datadir}"
echo "${ECHO_T}  sysconfdir: ${sysconfdir}"
echo "${ECHO_T}  docdir: ${docdir}"
echo "${ECHO_T}  localstatedir: ${localstatedir}"
echo "${ECHO_T}  includedir: ${includedir}"
echo "${ECHO_T}  libdir: ${libdir}"
echo ""
echo " Compiling Options:"
echo "${ECHO_T}  C Compiler: ${CC}"
echo "${ECHO_T}  C++ Compiler: ${CXX}"
echo "${ECHO_T}  CFLAGS: ${CFLAGS} ${AM_CFLAGS}"
echo "${ECHO_T}  CXXFLAGS: ${CXXFLAGS} ${AM_CXXFLAGS}"
echo "${ECHO_T}  LDFLAGS: ${LDFLAGS}"
echo "${ECHO_T}  Enable RPATH: ${osaf_cv_rpath}"

if test "$srcdir" == "."; then
        echo "${ECHO_T}  Using VPATH: no"
else
        echo "${ECHO_T}  Using VPATH: yes"
fi

AM_CONDITIONAL([WITH_VPATH], [test "$srcdir" != "."])

echo ""
echo " Dependencies Options:"

if test "$enable_hpi" = yes; then
        echo "${ECHO_T}  HPI_CFLAGS: ${HPI_CFLAGS}"
        echo "${ECHO_T}  HPI_LIBS: ${HPI_LIBS}"
fi

echo "${ECHO_T}  NETSNMP_CFLAGS: ${NETSNMP_CFLAGS}"
echo "${ECHO_T}  NETSNMP_LIBS: ${NETSNMP_LIBS}"
echo "${ECHO_T}  MIBDIRS: ${MIBDIRS}"
echo "${ECHO_T}  XML2_CFLAGS: ${XML2_CFLAGS}"
echo "${ECHO_T}  XML2_LIBS: ${XML2_LIBS}"
echo ""
echo " OpenSAF Options:"
echo "${ECHO_T}  Enable Java AIS Mapping: ${enable_java}"
echo "${ECHO_T}  Enable HPI support: ${enable_hpi}"
echo "${ECHO_T}  Enable OpenHPI: ${with_openhpi}"
echo "${ECHO_T}  Enable Building NCS smidump: ${enable_smidump}"
echo "======================================================="
echo ""
