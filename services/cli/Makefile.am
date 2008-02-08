if CONTROLLER

SUBDIRS = logstr

noinst_LTLIBRARIES = libcli.la

libcli_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ \
		-DNCS_CLI=1 -DNCSCLI_DEBUG=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 \
			-I ./inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libcli_la_LDFLAGS = -static 

libcli_la_SOURCES =	./src/cliapi.c \
			./src/cli.c \
			./src/clictree.c \
			./src/cliio.c \
			./src/clilog.c \
			./src/clitrav.c \
			./src/sysfcli.c \
			./src/prulegen/clilex.c \
			./src/prulegen/clipar.c

endif
