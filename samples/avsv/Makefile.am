bin_PROGRAMS = avsv_demo

avsv_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@
avsv_demo_SOURCES =	./avsv_amf.c \
			./avsv_app.c \
			./avsv_ckpt.c \
			./avsv_main.c

avsv_demo_LDADD =	@DEMO_LIBS@/libSaAmf.so \
			@DEMO_LIBS@/libsaf_common.so \
			@DEMO_LIBS@/libavsv_common.so \
			@DEMO_LIBS@/libSaCkpt.so
