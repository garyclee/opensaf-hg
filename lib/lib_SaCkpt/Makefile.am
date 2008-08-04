lib_LTLIBRARIES = libSaCkpt.la

libSaCkpt_la_SOURCES  = 

libSaCkpt_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPA=1 -DNCS_CPSV_LOG=1

libSaCkpt_la_LIBADD   =	$(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
			$(top_srcdir)/services/common/libsaf_common.la \
			$(top_srcdir)/services/cpsv/common/libcpsv_common.la \
			$(top_srcdir)/services/cpsv/cpa/libcpa.la
