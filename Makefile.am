#####################################################
# Top Level Makefile for OpenSAF
# Set rules for the following target deliverables:
# - Controller
# - Payload
# - 64 bit agent libraries
#####################################################


# Build smidump first (once) 
# Generate the header and miblib source files used by MIB applications

BUILT_SOURCES = build_smidump build_mapi

build_smidump:
	./tools/smidump/generate_smidump.sh

build_mapi:
	./tools/smidump/generate_mapi.sh

if AGENTS_LIB64

#Set the subdirs for the 64 bit agent libraries
#In the Current release the agents(linkable with user applications) are provided as 64-bit.

SUBDIRS = services/leap services/common services/mds services/vds/vda services/mab/oac services/dtsv/dta lib/lib_ncs_core services/avsv/ava services/avsv/common lib/lib_SaAmf services/cpsv/common services/cpsv/cpa lib/lib_SaCkpt services/avsv/cla lib/lib_SaClm services/edsv/common services/edsv/eda lib/lib_SaEvt services/glsv/gla lib/lib_SaLck lib/lib_SaLog services/mqsv/common services/mqsv/mqa lib/lib_SaMsg services/mab/mac services/mbcsv/src services/srmsv/srma services/ifsv/ifa services/ifsv/common

else

if CONTROLLER
# Generate the SNMP subagent,pssv integration files.
# The .so made out of these generated files are loaded by the servers.

BUILT_SOURCES += build_sources
build_sources:
	./tools/smidump/generate_files.sh

endif

# Now build the base code

SUBDIRS = 	services/common \
		services/leap \
		services/mds \
		services/vds/vda \
		services/dtsv/dta \
		services/dtsv/snmp_integ \
		services/mab/oac \
		services/mab/psr/snmp_integ \
		services/mab/mac \
		services/mqsv/mqa \
		services/mqsv/snmp_integ \
		services/mqsv/common \
		services/mbcsv/src \
		services/srmsv/srma \
		services/edsv/eda \
		services/edsv/snmp_integ \
		services/edsv/common\
		services/avsv/ava \
		services/avsv/common \
		services/avsv/cla \
		services/bam \
		services/cpsv/cpa \
		services/cpsv/snmp_integ \
		services/cpsv/common \
		services/glsv/gla \
		services/glsv/snmp_integ \
		services/glsv/common \
		services/ifsv/ifa \
		services/ifsv/common \
		services/ifsv/snmp_integ \
		services/ifsv/ipxs_snmp_integ \
		services/ifsv/entp_snmp_integ \
		services/ifsv/entp_ifsv_ir_snmp_integ \
		services/rde/rda \
		services/logsv/lga \
		lib/lib_ncs_core \
		lib/lib_SaAmf \
		lib/lib_SaCkpt \
		lib/lib_SaMsg \
		lib/lib_SaEvt \
		lib/lib_SaClm \
		lib/lib_SaLck \
		lib/lib_SaLog \
		tools/pdrbd \
		services
endif

rpm:install
	./rpms/create_opensaf_rpm.sh $(TARG_ARCH) $(BUILD_TYPE) $(LIB_SUFFIX) $(HW_MGMT_STR) $(MOT_ATCA_STR)

DISTCLEANFILES = `cat ./tools/smidump/generated_files_to_clean`

CLEANFILES = tools/smidump/files_generated  tools/smidump/mapi_generated
