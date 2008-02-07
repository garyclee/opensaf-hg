lib_LTLIBRARIES = libSaAmf.la

libSaAmf_la_SOURCES  = 
libSaAmf_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCS_AVA=1 -DNCS_AVSV_LOG=1
libSaAmf_la_LDFLAGS  =  -lpthread -ldl -lrt
libSaAmf_la_LIBADD   =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
			$(top_srcdir)/services/common/libsaf_common.la \
			$(top_srcdir)/services/avsv/ava/libava.la \
			$(top_srcdir)/services/avsv/common/libavsv_common.la
