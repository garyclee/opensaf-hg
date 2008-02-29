lib_LTLIBRARIES = libglsv_common.la

libglsv_common_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_GLSV_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/cpsv/inc/ \
			-I $(top_srcdir)/include/ 

libglsv_common_la_SOURCES =	./glsv_edu.c 
