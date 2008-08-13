bin_PROGRAMS = hisv_events_demo

hisv_events_demo_CPPFLAGS= -DNCS_SAF=1 \
		@DEMO_INC_DIR@ -I../../include

hisv_events_demo_SOURCES =	./hisv_events_demo_app.h \
				./hisv_events_demo_app.c \
				./hisv_events_demo_main.c

hisv_events_demo_LDADD =	@DEMO_LIBS@/libSaAmf.so \
				@DEMO_LIBS@/libedsv_common.so \
				@DEMO_LIBS@/libSaEvt.so \
				@DEMO_LIBS@/libhpl.so
