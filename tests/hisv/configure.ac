#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(hisv_test, 0.01)
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([src/hisv_power_ctrl.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(opensaf, 3.0)

AC_SUBST(TEST_INC_DIR, "-I /usr/include/opensaf -I /usr/include/openhpi")

AC_ARG_VAR(saflib_suffix, [set this to 64 if opensaf libraries are present in a  .../lib64 directory])
#AC_SUBST(TEST_LIBS, "../../../targets/$host/lib${saflib_suffix}")

#mkdir -p $PWD/bin/$host
#bindir=$PWD/bin/$host

#mkdir -p $PWD/lib/$host
#libdir=$PWD/lib/$host

#############################################
# Set Cross Compile options if any
#############################################

AC_ARG_VAR(cc_exec_prefix, [Specify cross-compiler toolchain executables prefix Eg. For ppc_74xx-gcc specify cc_exec_prefix=ppc_74xx])

AC_ARG_VAR(cc_libs_dir, [Specify cross-compiled libraries path to be included with -L])
	
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
if ! test "$cc_libs_dir" = ""
then
	export LDFLAGS="$LDFLAGS -L$cc_libs_dir" 
fi

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
# Checks for libraries.
#############################################

#############################################
# Checks for header files.
#############################################
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
#AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

AC_CHECK_HEADERS([malloc.h stddef.h stdlib.h string.h unistd.h])

#############################################
# Checks for typedefs, structures, and compiler characteristics.
#############################################
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

#############################################
# Checks for library functions.
#############################################
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([isascii memset strchr strerror strpbrk strrchr strspn strstr strtol])

#############################################
# List the output Makefiles
#############################################
AC_CONFIG_FILES([Makefile
		src/Makefile])
AC_OUTPUT
