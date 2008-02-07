bin_PROGRAMS = cpsv_demo

cpsv_demo_CPPFLAGS= -DNCS_SAF \
		@DEMO_INC_DIR@

cpsv_demo_SOURCES =	./cpsv_main_app.c \
			./cpsv_test_app.c

cpsv_demo_LDADD =	@DEMO_LIBS@/libncs_core.so  \
					@DEMO_LIBS@/libSaCkpt.so
