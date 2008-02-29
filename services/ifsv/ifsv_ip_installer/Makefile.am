bin_PROGRAMS = ncs_ifsv_ip_installer

ncs_ifsv_ip_installer_CPPFLAGS= -DNCS_SAF=1 -DNCS_IFSV_BOND_INTF=1 \
		-I $(top_srcdir)/include/

ncs_ifsv_ip_installer_SOURCES = ./ifsv_app.h \
		./ifsv_demo.h \
		./drv_samp.c \
		./ifsv_demo.c \
		./ifsv_app.c

ncs_ifsv_ip_installer_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                ../common/libifsv_common.la \
                ../ifa/libifa.la 

