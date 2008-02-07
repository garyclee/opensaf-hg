#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(opensaf, 1.0, users@list.opensaf.org)
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([samples/cli/cli_demo_app.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(opensaf, 1.0)

#############################################
# Get input for enabling hardware management
#############################################
AC_ARG_VAR(hw_mgmt, [To build for hardware management(OpenHPI) '1' HPI B.02.01, '2' HPI B.01.01, '3' HPI A.01.01 complaint])
AM_CONDITIONAL([HW_MGMT], [test "$hw_mgmt" != ""])

HPI_DEF=""
HW_MGMT_STR=""
if test "$hw_mgmt" = "1"
then
   AC_SUBST(HPI_LIB, "openhpi")
   HPI_DEF=-DHPI_B_02
   HW_MGMT_STR="hw_mgmt"
elif test "$hw_mgmt" = "2"
then
   AC_SUBST(HPI_LIB, "openhpi")
   HW_MGMT_STR="hw_mgmt"
elif test "$hw_mgmt" = "3"
then
   AC_SUBST(HPI_LIB, "openhpiclient")
   HPI_DEF=-DHPI_A
   HW_MGMT_STR="hw_mgmt"
elif test "$hw_mgmt" != ""
then
   echo "Error: Invalid input 'hw_mgmt=$hw_mgmt', set hw_mgmt=1/2/3 to interface with OpenHPI hardware management"
   echo "       --> '1' HPI B.02.01 spec complaint" 
   echo "       --> '2' HPI B.01.01 spec complaint" 
   echo "       --> '3' HPI A.01.01 spec complaint" 
   exit
fi

#############################################
# Get input for enabling platform interface
#############################################
AC_ARG_VAR(mot_atca, [Use mot_atca=1 if platform interface is needed])
AM_CONDITIONAL([MOT_ATCA], [test "$mot_atca" = "1"])

if test "$mot_atca" = "1"
then
    MOT_ATCA_VAL=1
    MOT_ATCA_STR="mot_atca"
    NETSNMP_5_2_2=1
else
    MOT_ATCA_VAL=0
    MOT_ATCA_STR=""
    NETSNMP_5_2_2=0
fi

AC_SUBST(MOT_ATCA_STR)
AC_SUBST(HW_MGMT_STR)


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
AC_SUBST( NCS_CORE_CPP_DEFINES, "-DNCS_NID=1 -DNCS_MTREE_PAT_STYLE_SUPPORTED=1 -DNCS_MMGR_DEBUG=$DEBUG_VAL -DENABLE_LEAP_DBG=$DEBUG_VAL -DNCSMDS_DEBUG=$DEBUG_VAL -DNCS_IPV6=0 -DNCS_IFSV_IPXS=1 -DNCS_IFSV_USE_NETLINK=1 -DNCS_TMR_DBG_ENABLE=$DEBUG_VAL -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 -DFUND_DEBUG_TEST=0 -DNCS_MONC_CPSV_USE=0 -DNCS_SAF=1 -DNCS_RMS=0 -DNCS_MAB=1 -DMOT_ATCA=$MOT_ATCA_VAL -DNET_SNMP_5_2_2_SUPPORT=$NETSNMP_5_2_2  -DNCS_MDS=1 -DNCSMDS_LOG=1 -DDTS_SIM_TEST_ENV=1 -D__JSE_LINUX__ -D__JSE_UNIX__ -DHJ_LINUX=1 -DUSE_READLINE -DDTA_FLOW=1 $DEBUG_FLAG $HPI_DEF")

# Sets variable specific to host & target (got via --target=...)

#############################################
# Get input for build_type
#############################################
AC_ARG_VAR(build_type, [Specify the type of build (controller/payload/64bit_agent_libs)])
AM_CONDITIONAL([CONTROLLER], [test "$build_type" = "controller"])
AM_CONDITIONAL([AGENTS_LIB64], [test "$build_type" = "64bit_agent_libs"])

#############################################
# Set install path (libdir and bindir) for libraries and binaries
#############################################
AC_ARG_VAR(libdir_suffix, [Set this to 64 if library install directory needs to be .../lib64 ]) 

case $target_cpu in 
     x86_64|ia64|mips64|ppc64|sparc64|parisc64|s390x) # As of now, some known values
           LIB_SUFFIX=lib64
           ;;
     *)
           LIB_SUFFIX=lib
           ;;
esac

AC_SUBST(LIB_SUFFIX)

if test "$build_type" = "controller";
then
	echo "Building for build_type=controller"
	mkdir -p $PWD/targets/$host/controller
        mkdir -p $PWD/targets/$host/$LIB_SUFFIX
	bindir=$PWD/targets/$host/controller
        libdir=$PWD/targets/$host/$LIB_SUFFIX
 
        CFLAGS="$CFLAGS -fPIC -Wall -Wmissing-prototypes -Wno-pointer-sign -Werror" 
        CXXFLAGS="$CXXFLAGS -fPIC -Wall -Wno-deprecated -Wno-write-strings -Werror"
elif test "$build_type" = "payload";
then
	echo "Building for build_type=payload"
        mkdir -p $PWD/targets/$host/payload
        mkdir -p $PWD/targets/$host/$LIB_SUFFIX
        bindir=$PWD/targets/$host/payload
        libdir=$PWD/targets/$host/$LIB_SUFFIX
        CFLAGS="$CFLAGS -fPIC -Wall -Wmissing-prototypes -Wno-pointer-sign -Werror"
        CXXFLAGS="$CXXFLAGS -fPIC -Wall -Wno-deprecated -Wno-write-strings -Werror"
elif test "$build_type" = "64bit_agent_libs";
then
        LIB_SUFFIX=lib64
  	case $host_cpu in
    		i686*|i386*|arm*|m68k|ns32k|sh3*|sparc|vax)
                echo "********************************************************************"
                echo "Warning: You are trying to configure 64-bit build on a 32-bit machine"
                echo "Please Specify correct --host value if you are cross compiling"  
                echo "********************************************************************"
                ;;
  	esac

        echo "Building 64bit agent libraries"
        mkdir -p $PWD/targets/$host/$LIB_SUFFIX
        libdir=$PWD/targets/$host/$LIB_SUFFIX
        CFLAGS="$CFLAGS -fPIC"
        CXXFLAGS="$CXXFLAGS -fPIC"
else
	echo "You must specify build_type=[payload/controller/64bit_agent_libs]"
	exit 1
fi

###################################
# Export variables for use by rpms
###################################
TARG_ARCH=$host
AC_SUBST(TARG_ARCH)

BUILD_TYPE=$build_type
AC_SUBST(BUILD_TYPE)


#############################################
# Set Cross Compile options if any
#############################################

AC_ARG_VAR(cc_exec_prefix, [Specify cross-compiler toolchain executables prefix Eg. For ppc_74xx-gcc specify cc_exec_prefix=ppc_74xx])

AC_ARG_VAR(cc_lib_dir, [Specify cross-compiled libraries path to be included with -L])
	
#########################################
# Exporting CC variables
#########################################
if ! test "$cc_exec_prefix" = ""
then
	export AR="$cc_exec_prefix-ar"
	export AS="$cc_exec_prefix-as"
	export CC="$cc_exec_prefix-gcc"
	export CPP="$cc_exec_prefix-cpp"
	export CXX="$cc_exec_prefix-g++"
	export CXXCPP="$cc_exec_prefix-g++ -E" 
	export LD="$cc_exec_prefix-ld"
	export RANLIB="$cc_exec_prefix-ranlib" 
fi
if ! test "$cc_lib_dir" = ""
then
	export LDFLAGS="$LDFLAGS -L$cc_lib_dir" 
fi

# Disabling Optimization
export CFLAGS="$CFLAGS -g -O0"

#############################################
# Checks for programs.
#############################################
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PROG_LIBTOOL

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
if test "$mds_arch" = ""
then
    mdsarchtype=0
else
    mdsarchtype=$mds_arch
fi
AC_SUBST( MDS_WORD_ARCH_FLAGS," -DMDS_ARCH_TYPE=$mdsarchtype" )

#############################################
# Checks for libraries.
#############################################

#############################################
# Checks for header files.
#############################################
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

#############################################
# Checks for typedefs, structures, and compiler characteristics.
#############################################
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

#############################################
# Checks for library functions.
#############################################
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([clock_gettime ftruncate gettimeofday isascii localtime_r memset munmap pow select setenv socket strcasecmp strchr strerror strpbrk strrchr strspn strstr strtol])

#############################################
# List the output Makefiles
#############################################
AC_CONFIG_FILES([Makefile
                 lib/lib_SaAmf/Makefile
                 lib/lib_SaCkpt/Makefile
                 lib/lib_SaMsg/Makefile
                 lib/lib_SaEvt/Makefile
                 lib/lib_SaClm/Makefile
                 lib/lib_SaLck/Makefile
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
                 services/ifsv/entp_vip_snmp_integ/Makefile
                 services/ifsv/ifa/Makefile
                 services/ifsv/ifd/Makefile
                 services/ifsv/ifnd/Makefile
                 services/ifsv/ifsv_ip_installer/Makefile
                 services/ifsv/ipxs_snmp_integ/Makefile
                 services/ifsv/logstr/Makefile
                 services/ifsv/snmp_integ/Makefile
                 services/leap/Makefile
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
                 tools/pdrbd/Makefile
                 tools/pdrbd/logstr/Makefile
                 tools/pdrbd/src/Makefile])

AC_OUTPUT

