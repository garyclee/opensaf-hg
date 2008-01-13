bin_PROGRAMS = dtsv_demo
lib_LTLIBRARIES = libdummy_logstr.la

dtsv_demo_CPPFLAGS= -DNCS_SAF=1 -DNCS_DTS=1 \
		@DEMO_INC_DIR@

dtsv_demo_SOURCES =	./dtsv_dummy_ss.h \
					./dtsv_demo_main.c \
					./dtsv_dummy_ss1.c

dtsv_demo_LDADD = @DEMO_LIBS@/libncs_core.so


libdummy_logstr_la_CPPFLAGS= -m32 -DNCS_SAF=1 -DNCS_DTS=1 \
                              @DEMO_INC_DIR@

libdummy_logstr_la_SOURCES =   ./dtsv_dummy1_str.c 

libdummy_logstr_la_LIBADD = @DEMO_LIBS@/libncs_core.so 

libdummy_logstr_la_LDFLAGS = -m32 -version-number 2

