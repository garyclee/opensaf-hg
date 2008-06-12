lib_LTLIBRARIES = libavsv_logstr.la

libavsv_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_DTS=1 \
	-I ../inc/ \
	-I $(top_srcdir)/services/leap/inc/ \
	-I $(top_srcdir)/services/mds/inc/ \
	-I $(top_srcdir)/services/mab/inc/ \
	-I $(top_srcdir)/services/common/inc/ \
	-I $(top_srcdir)/include/ 

libavsv_logstr_la_SOURCES =	./ava_logstr.c \
				./avd_logstr.c \
				./avnd_logstr.c \
				./avsv_logstr.c \
				./cla_logstr.c

libavsv_logstr_la_LDFLAGS = -version-number 2
