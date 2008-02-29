bin_PROGRAMS = edsv_demo

edsv_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@

edsv_demo_SOURCES =	./edsv_demo_app.h \
			./edsv_demo_app.c \
			./edsv_demo_main.c

edsv_demo_LDADD =	@DEMO_LIBS@/libSaAmf.so \
			@DEMO_LIBS@/libedsv_common.so \
			@DEMO_LIBS@/libSaEvt.so
