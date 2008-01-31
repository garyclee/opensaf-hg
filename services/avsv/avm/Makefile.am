SUBDIRS = logstr pssv_integ snmp_integ

noinst_LTLIBRARIES = libavm.la

libavm_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_AVM=1 -DNCS_AVM_LOG=1 \
		-I ../inc/ \
		-I ../inc/avsv_snmp_inc/ \
		-I ./inc/ \
		-I ./inc/avm_snmp_inc \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/hisv/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libavm_la_LDFLAGS =	-static

libavm_la_SOURCES =	./src/avm_avd.c \
			./src/avm_chkop.c \
			./src/avm_ckpt_dec.c \
			./src/avm_ckpt_edu.c \
			./src/avm_ckpt_enc.c \
			./src/avm_ckpt_updt.c \
			./src/avm_db.c \
			./src/avm_dblog.c \
			./src/avm_dhcp_util.c \
			./src/avm_eda.c \
			./src/avm_ent.c \
			./src/avm_evt.c \
			./src/avm_fsm.c \
			./src/avm_fund.c \
			./src/avm_hpi_util.c \
			./src/avm_init.c \
			./src/avm_lfm.c \
			./src/avm_mab.c \
			./src/avm_mds.c \
			./src/avm_proc.c \
			./src/avm_rda.c \
			./src/avm_role_evt.c \
			./src/avm_scalar.c \
			./src/avm_tmr.c \
			./src/avm_traps.c \
			./src/avm_upgd_traps.c \
			./src/avm_util.c \
			./src/avm_miblib_gen/ncsavmentdeploytableentry_mib.c \
			./src/avm_miblib_gen/ncsavmentfaultdomaintableentry_mib.c \
			./src/avm_miblib_gen/ncsavmentupgradetableentry_mib.c \
			./src/avm_miblib_gen/ncsavmscalars_mib.c 

if MOT_ATCA
libavm_la_LIBADD = $(top_srcdir)/services/rde/rda/librda.la \
                   $(top_srcdir)/services/avsv/avm/liblfm_avm_intf.la
else
libavm_la_LIBADD = $(top_srcdir)/services/rde/rda/librda.la
endif

if MOT_ATCA

lib_LTLIBRARIES = liblfm_avm_intf.la

liblfm_avm_intf_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_AVM=1 -DNCS_AVM_LOG=1 \
		-I ../inc/ \
		-I ../inc/avsv_snmp_inc/ \
		-I ./inc/ \
		-I ./inc/avm_snmp_inc \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/hisv/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

liblfm_avm_intf_la_SOURCES =	./src/lfm_avm_intf.c 

endif

