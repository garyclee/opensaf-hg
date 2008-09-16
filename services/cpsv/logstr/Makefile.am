lib_LTLIBRARIES = libcpsv_logstr.la

libcpsv_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_EDSV_LOG=1 -DNCS_DTS=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include

libcpsv_logstr_la_SOURCES =	./cpa_logstr.c \
				./cpd_logstr.c \
				./cpnd_logstr.c

libcpsv_logstr_la_LDFLAGS = -version-number 3
