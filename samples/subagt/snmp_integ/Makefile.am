lib_LTLIBRARIES = libsnmptm_subagt.la

libsnmptm_subagt_la_CPPFLAGS= -DNCS_SAF=1 \
            -I ../inc/ \
	@DEMO_INC_DIR@

libsnmptm_subagt_la_SOURCES =	./mcg_ncs_test_mib_agt_stub.h \
								./snmptm_subagt_init.h \
								./snmptm_subagt_init.c \
								./mcg_ncs_test_mib_agt_stub.c

libsnmptm_subagt_la_LIBADD = @DEMO_LIBS@/libncs_core.so \
                    @DEMO_LIBS@/libsaf_common.so \
                    @DEMO_LIBS@/libSaEvt.so \
                    @DEMO_LIBS@/libedsv_common.so
