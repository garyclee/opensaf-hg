bin_PROGRAMS = mbcsv_demo

mbcsv_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@

mbcsv_demo_SOURCES = ./mbcsv_demo_app.h \
		./mbcsv_demo_app.c \
		./mbcsv_demo_main.c

mbcsv_demo_LDADD =	@DEMO_LIBS@/libncs_core.so \
			@DEMO_LIBS@/libmbca.so
