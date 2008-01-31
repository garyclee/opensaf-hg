#      -*- OpenSAF  -*-
#
#  Copyright (c) 2007, Ericsson AB
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under High-Availability Operating 
# Environment Software License Version 1.4.
# Complete License can be accesseble from below location.
# http://www.opensaf.org/license 
# See the Copying file included with the OpenSAF distribution for
# full licensing terms.
#
# Author(s):
#   
#

aishello := Hello/aishello/aishello

ifeq ($(TIPCUTILSVER),)
	TIPCUTILSVER := 1.0.4
endif

ifeq ($(TIPCUTILSURL),)
	TIPCUTILSURL := http://prdownloads.sourceforge.net/tipc/tipcutils-$(TIPCUTILSVER).tar.gz
endif

ROOT_CONTROLLER_DEPS := 		\
	$(OPENSAF_CONTROLLER_RPM) 	\
	$(aishello) 			\
	archive/* 			\
	archive/scripts/controller/*	\
	tipcutils-$(TIPCUTILSVER)/tipc-config

ROOT_PAYLOAD_DEPS := 			\
	$(OPENSAF_PAYLOAD_RPM) 		\
	$(aishello) 			\
	archive/* 			\
	archive/scripts/payload/*	\
	tipcutils-$(TIPCUTILSVER)/tipc-config

INSTALL_DIR := install/opensaf_cluster_sim

all: root.controller/root.cpio root.payload/root.cpio
	make -C uml all

root.controller/root.cpio: $(ROOT_CONTROLLER_DEPS)
	rm -rf root.controller
	./bin/install.sh controller

root.payload/root.cpio: $(ROOT_PAYLOAD_DEPS)
	rm -rf root.payload
	./bin/install.sh payload

$(aishello):
	make -C Hello/aishello

tipcutils-$(TIPCUTILSVER): archive/tipcutils-$(TIPCUTILSVER).tar.gz
	tar -xzf $<

tipcutils-$(TIPCUTILSVER)/tipc-config: tipcutils-$(TIPCUTILSVER) uml/linux-$(KVER)
	make KERNELDIR=../uml/linux-$(KVER) -C $<

archive/tipcutils-$(TIPCUTILSVER).tar.gz:
	mkdir -p archive
	wget -O $@ $(TIPCUTILSURL)

uml/linux-$(KVER):
	make -C uml all

install: root.controller/root.cpio root.payload/root.cpio
	rm -rf $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/bin
	cp bin/mklndircpio $(INSTALL_DIR)/bin
	cp archive/Makefile.dev_env $(INSTALL_DIR)/Makefile
	cp archive/README.dev_env $(INSTALL_DIR)/README
	mkdir -p $(INSTALL_DIR)/etc
	cp -a etc $(INSTALL_DIR)
	cp opensaf.sh $(INSTALL_DIR)
	cp -a root.controller $(INSTALL_DIR)
	cp -a root.payload $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/uml/bin
	cp -a uml/bin/linux $(INSTALL_DIR)/uml/bin
	cp -a uml/bin/uml* $(INSTALL_DIR)/uml/bin
	mkdir -p $(INSTALL_DIR)/include
	cp ../../include/sa*.h $(INSTALL_DIR)/include
	mkdir -p $(INSTALL_DIR)/lib/$(ARCH)
	cp -a root.controller/opt/opensaf/controller/lib/libSa* $(INSTALL_DIR)/lib/$(ARCH)
	cp -a root.controller/opt/opensaf/controller/lib/libncs_core.* $(INSTALL_DIR)/lib/$(ARCH)
	cp -a root.controller/opt/opensaf/controller/lib/lib*_common.* $(INSTALL_DIR)/lib/$(ARCH)

dev_env: install
	tar --directory=install -czf cluster_sim_uml_dev_env.tgz .

clean:
	rm -rf tipcutils-$(TIPCUTILSVER)
	rm -rf root.controller root.payload
	rm -rf repl-opensaf
	rm -rf install
	rm -f cluster_sim_uml_dev_env.tgz
	make -C uml clean
	make -C Hello/aishello clean

realclean: clean
	rm -rf archive/tipcutils-$(TIPCUTILSVER).tar.gz
	make -C uml realclean

distclean: realclean
