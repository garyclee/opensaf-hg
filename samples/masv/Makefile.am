bin_PROGRAMS = maa_demo trap_demo

maa_demo_CPPFLAGS= -DNCS_SAF=1 -DNCS_DEBUG=1 \
		@DEMO_INC_DIR@ \
		-I ../subagt/inc

maa_demo_SOURCES =	./maa_api_usage.cpp

maa_demo_LDADD = @DEMO_LIBS@/libncs_core.so \
				 @DEMO_LIBS@/libmaa.so

trap_demo_CPPFLAGS= -DNCS_SAF=1 -DNCS_DEBUG=1 \
		@DEMO_INC_DIR@ \
		-I ../subagt/inc

trap_demo_SOURCES =  ./ncs_trap_usage.cpp

trap_demo_LDADD = @DEMO_LIBS@/libncs_core.so \
                 @DEMO_LIBS@/libSaEvt.so
