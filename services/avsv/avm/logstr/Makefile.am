lib_LTLIBRARIES = libavm_logstr.la

libavm_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_DTS=1 -I ../inc/ \
	-I $(top_srcdir)/services/leap/inc/ \
	-I $(top_srcdir)/services/mds/inc/ \
	-I $(top_srcdir)/services/mab/inc/ \
	-I $(top_srcdir)/services/common/inc/ \
	-I $(top_srcdir)/include/ 

libavm_logstr_la_SOURCES =	./avm_logstr.c

libavm_logstr_la_LDFLAGS = -version-number 1
