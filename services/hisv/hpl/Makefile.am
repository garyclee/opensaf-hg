lib_LTLIBRARIES = libhpl.la

libhpl_la_CPPFLAGS =  	@NCS_CORE_CPP_DEFINES@ \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/  

libhpl_la_SOURCES =	./hpl_init.c \
			./hpl_api.c \
			./hpl_mds.c 
