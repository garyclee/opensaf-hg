bin_PROGRAMS = pxy_pxd_demo

pxy_pxd_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@
pxy_pxd_demo_SOURCES =	./src/pxy_pxd_amf.c \
			./src/pxy_pxd_main.c

pxy_pxd_demo_LDADD =	@DEMO_LIBS@/libSaAmf.so \
			@DEMO_LIBS@/libsaf_common.so \
			@DEMO_LIBS@/libavsv_common.so 
