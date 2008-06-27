if CONTROLLER
lib_LTLIBRARIES = libifsv_entp_vip_subagt.la

libifsv_entp_vip_subagt_la_CPPFLAGS = 	-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/include/ \
				-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0 \
				@NCS_CORE_CPP_DEFINES@ 
endif
