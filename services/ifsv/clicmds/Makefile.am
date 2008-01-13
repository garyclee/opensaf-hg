lib_LTLIBRARIES = libifsv_clicef.la

libifsv_clicef_la_CPPFLAGS = 	@NCS_CORE_CPP_DEFINES@ \
				-DNCS_IFSV_BOND_INTF=1 -DNCS_MAB=1 \
				-DNCS_SAF=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 \
				-DNCS_CLI=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 \
				-DNCS_IFSV_IPXS=1 \
				-I ../inc/ \
				-I ../inc/ifsv_snmp_inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
				-I $(top_srcdir)/services/cli/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/ 

libifsv_clicef_la_SOURCES = 	./ifsv_cli.c \
				./vip_cli.c 
