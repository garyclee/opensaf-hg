lib_LTLIBRARIES = libedsv_logstr.la

libedsv_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_EDSV_LOG=1 -DNCS_DTS=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include

libedsv_logstr_la_SOURCES =	./eds_logstr.c \
				./eda_logstr.c

libedsv_logstr_la_LDFLAGS = -version-number 2
