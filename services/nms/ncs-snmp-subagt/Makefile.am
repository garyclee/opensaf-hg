SUBDIRS = logstr

lib_LTLIBRARIES = libncs_snmp_subagt.la

libncs_snmp_subagt_la_CPPFLAGS =    -I ./inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
			    -I $(top_srcdir)/include/ \
			    -I $(top_srcdir)/services/common/inc/ \
			    -I $(top_srcdir)/services/avsv/snmp_integ/\
			    -I $(top_srcdir)/services/ifsv/snmp_integ/\
			    -I $(top_srcdir)/services/ifsv/ipxs_snmp_integ\
			    -I $(top_srcdir)/services/ifsv/entp_snmp_integ/\
			    -I $(top_srcdir)/services/dtsv/snmp_integ/\
			    -I $(top_srcdir)/services/avsv/avm/snmp_integ\
                -DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1  \
				-DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DNCS_EDA=1 \
			    @NCS_CORE_CPP_DEFINES@ 

libncs_snmp_subagt_la_SOURCES = ./src/subagt_oid_db.c \
                            ./src/subagt_agt.c \
                            ./src/subagt_amf.c \
                            ./src/subagt_eda.c \
                            ./src/subagt_edu.c \
                            ./src/subagt_mab.c \
                            ./src/subagt_mac.c \
                            ./src/subagt_mbx.c \
                            ./src/subagt_log.c \
                            ./src/subagt_dlapi.c \
                            ./src/subagt_cb.c 

libncs_snmp_subagt_la_LDFLAGS = $(shell net-snmp-config --agent-libs)

libncs_snmp_subagt_la_LIBADD = \
   $(top_srcdir)/services/avsv/avm/snmp_integ/libavm_subagt.la \
   $(top_srcdir)/services/ifsv/entp_snmp_integ/libifsv_entp_subagt.la \
   $(top_srcdir)/services/ifsv/ipxs_snmp_integ/libipxs_subagt.la \
   $(top_srcdir)/services/ifsv/snmp_integ/libifsv_subagt.la  \
   $(top_srcdir)/services/avsv/snmp_integ/libavsv_subagt.la \
   $(top_srcdir)/services/dtsv/snmp_integ/libdtsv_subagt.la \
   $(top_srcdir)/services/rde/rda/librda.la
