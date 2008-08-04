bin_PROGRAMS = glsv_demo

glsv_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@
glsv_demo_SOURCES = ./glsv_demo_app1.c \
		./glsv_main_app1.c \
		./glsv_test_app.c

glsv_demo_LDADD =	@DEMO_LIBS@/libncs_core.so \
					@DEMO_LIBS@/libSaLck.so
