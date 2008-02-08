lib_LTLIBRARIES = libcpsv_common.la

libcpsv_common_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPND=1 -DNCS_CPD=1\
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libcpsv_common_la_SOURCES =	./cpsv_edu.c \
				./cpsv_evt.c \
				./cpsv_mbedu.c
