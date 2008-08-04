noinst_LTLIBRARIES = libavd.la

libavd_la_CPPFLAGS =	 @NCS_CORE_CPP_DEFINES@  -DNCS_AVD=1 -DNCS_AVSV_LOG=1 \
			-I ../inc/ \
			-I ../inc/avsv_snmp_inc \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libavd_la_LDFLAGS =	-static

libavd_la_SOURCES =	./avd_avm.c \
			./avd_avmmsg.c \
			./avd_avnd.c \
			./avd_bam.c \
			./avd_bammsg.c \
			./avd.c \
			./avd_chkmsg.c \
			./avd_chkop.c \
			./avd_ckpt_dec.c \
			./avd_ckpt_edu.c \
			./avd_ckpt_enc.c \
			./avd_ckpt_updt.c \
			./avd_comp.c \
			./avd_csi.c \
			./avd_dblog.c \
			./avd_dmsg.c \
			./avd_evtlog.c \
			./avd_hb.c \
			./avd_hlt.c \
			./avd_hpi.c \
			./avd_mab.c \
			./avd_mds.c \
			./avd_mibtbl.c \
			./avd_ndfsm.c \
			./avd_ndmsg.c \
			./avd_ndproc.c \
			./avd_pg.c \
			./avd_proc.c \
			./avd_role.c \
			./avd_sg2Nfsm.c \
			./avd_sg.c \
			./avd_sgNAcvRfsm.c \
			./avd_sgNoRfsm.c \
			./avd_sgNpMfsm.c \
			./avd_sgNWayfsm.c \
			./avd_sgproc.c \
			./avd_si.c \
			./avd_su.c \
			./avd_susi.c \
			./avd_tmr.c \
			./avd_trap.c \
			./avd_utilmsg.c \
			./avd_miblib_gen/avsvscalars_mib.c \
			./avd_miblib_gen/ncsndtableentry_mib.c \
			./avd_miblib_gen/ncssgtableentry_mib.c \
			./avd_miblib_gen/ncssitableentry_mib.c \
			./avd_miblib_gen/ncssutableentry_mib.c \
			./avd_miblib_gen/saamfcompcstypesupportedtableentry_mib.c \
			./avd_miblib_gen/saamfcomptableentry_mib.c \
			./avd_miblib_gen/saamfcsinamevaluetableentry_mib.c \
			./avd_miblib_gen/saamfcsitableentry_mib.c \
			./avd_miblib_gen/saamfcstypeparamentry_mib.c \
			./avd_miblib_gen/saamfhealthchecktableentry_mib.c \
			./avd_miblib_gen/saamfnodetableentry_mib.c \
			./avd_miblib_gen/saamfscalars_mib.c \
			./avd_miblib_gen/saamfsgsirankentry_mib.c \
			./avd_miblib_gen/saamfsgsurankentry_mib.c \
			./avd_miblib_gen/saamfsgtableentry_mib.c \
			./avd_miblib_gen/saamfsisideptableentry_mib.c \
			./avd_miblib_gen/saamfsitableentry_mib.c \
			./avd_miblib_gen/saamfsusitableentry_mib.c \
			./avd_miblib_gen/saamfsuspersirankentry_mib.c \
			./avd_miblib_gen/saamfsutableentry_mib.c \
			./avd_miblib_gen/saclmnodetableentry_mib.c \
			./avd_miblib_gen/safclmscalarobject_mib.c
