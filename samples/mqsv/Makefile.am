bin_PROGRAMS = mqsv_demo

mqsv_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@

mqsv_demo_SOURCES =	./mqsv_demo_app.c \
			./mqsv_main_app.c

mqsv_demo_LDADD =	@DEMO_LIBS@/libncs_core.so \
				 	@DEMO_LIBS@/libSaMsg.so \
					@DEMO_LIBS@/libsaf_common.so \
					@DEMO_LIBS@/libmqsv_common.so
