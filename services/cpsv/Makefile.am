if CONTROLLER

SUBDIRS = cpd cpnd logstr 
bin_PROGRAMS = ncs_cpd ncs_cpnd

else

SUBDIRS = cpnd 
bin_PROGRAMS = ncs_cpnd

endif


ncs_cpd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPD=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/

ncs_cpnd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPND=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/

ncs_cpd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c	

ncs_cpd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/lib/lib_SaClm/libSaClm.la \
		$(top_srcdir)/services/mbcsv/src/libmbca.la \
		./cpd/libcpd.la \
		./common/libcpsv_common.la

ncs_cpnd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c	

ncs_cpnd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/lib/lib_SaClm/libSaClm.la \
		./cpnd/libcpnd.la \
		./common/libcpsv_common.la

