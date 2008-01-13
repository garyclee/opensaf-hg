lib_LTLIBRARIES = libSaEvt.la

libSaEvt_la_SOURCES = 

libSaEvt_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_EDA=1 -DNCS_EDSV_LOG=1

libSaEvt_la_LIBADD   =	$(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
			$(top_srcdir)/services/common/libsaf_common.la \
			$(top_srcdir)/services/edsv/common/libedsv_common.la \
			$(top_srcdir)/services/edsv/eda/libeda.la
