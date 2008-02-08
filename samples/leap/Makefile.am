bin_PROGRAMS = leaptest_demo
lib_LTLIBRARIES = liblttest_dl_app.la 


leaptest_demo_CPPFLAGS= -DNCS_SAF=1 -DNCS_IFSV_BOND_INTF=1 \
			@DEMO_INC_DIR@

liblttest_dl_app_la_CPPFLAGS= -DNCS_SAF=1 -DNCS_IFSV_BOND_INTF=1 \
                        @DEMO_INC_DIR@

leaptest_demo_SOURCES = ./leaptest.h \
					./test_atomic_counts.c \
					./test_bufmgr.c \
					./test_encdec.c \
					./test_file.c \
					./test_locks.c \
					./test_mailbox.c \
					./test_main.c \
					./test_memdiag.c \
					./test_patricia.c \
					./test_processlib.c \
					./test_timers.c 
leaptest_demo_LDADD = @DEMO_LIBS@/libncs_core.so

liblttest_dl_app_la_SOURCES = ./test_dl_app.c
