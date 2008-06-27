lib_LTLIBRARIES = libglsv_logstr.la

libglsv_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_GLSV_LOG=1 -DNCS_DTS=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/dtsv/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include

libglsv_logstr_la_SOURCES =	./gla_logstr.c \
			./gld_logstr.c \
			./glnd_logstr.c

libglsv_logstr_la_LDFLAGS = -version-number 3
