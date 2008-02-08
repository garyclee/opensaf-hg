bin_PROGRAMS = srmsv_demo

srmsv_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@

srmsv_demo_SOURCES = ./srmsv_demo.h \
		./srmsv_demo.c \
		./srmsv_main_app.c

srmsv_demo_LDADD =	@DEMO_LIBS@/libncs_core.so  \
					@DEMO_LIBS@/libsrma.so
