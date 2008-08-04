noinst_LTLIBRARIES = libglnd.la

libglnd_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_GLND=1 -DNCS_GLSV_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/ 

libglnd_la_LDFLAGS = -static

libglnd_la_SOURCES =	./glnd_agent.c \
			./glnd_amf.c \
			./glnd_api.c \
			./glnd_cb.c \
			./glnd_ckpt.c \
			./glnd_client.c \
			./glnd_evt.c \
			./glnd_log.c \
			./glnd_mds.c \
			./glnd_queue.c \
			./glnd_res.c \
			./glnd_res_req.c \
			./glnd_restart.c \
			./glnd_shm.c \
			./glnd_tmr.c
