# -*- Makefile -*-

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

# Makefile for the safsource UML target. The UML executables contains
# a root "ramfs". The start is very generic and the UML executables
# can be used for general purpose.


ifeq ($(KVER),)
	KVER := 2.6.18.6
endif

ifeq ($(KURL),)
	KURL := http://www.kernel.org/pub/linux/kernel/v2.6/linux-$(KVER).tar.bz2
endif

ifeq ($(BBVER),)
	BBVER := 1.3.2
endif

ifeq ($(BBURL),)
	BBURL := http://busybox.net/downloads/busybox-$(BBVER).tar.bz2
endif

ifeq ($(UMLUTILSVER),)
	UMLUTILSVER := 20040406
endif

ifeq ($(UMLUTILSURL),)
	UMLUTILSURL := http://prdownloads.sourceforge.net/user-mode-linux/uml_utilities_$(UMLUTILSVER).tar.bz2
endif

TOP := $(shell pwd)
UID := $(shell id -u)
GID := $(shell id -g)

.PHONY: help all archive linux menuconfig clean realclean

help:
	@echo "Valid targets:"
	@echo "  all - Build everything"
	@echo "  linux - Rebuild with updated root (use this!)"
	@echo "  menuconfig - Change kernel configuration"
	@echo "  archive - Download linux-kernel and busybox archives"
	@echo "  clean - Remove everything EXCEPT the archive/ dir"
	@echo "  realclean - Remove everything"
	@echo "Current versions; linux-$(KVER) busybox-$(BBVER)"
	@echo "Run [make linux] *twice* if you have updated the module config!"

all: bin/linux bin/uml_switch

uml_utilities_$(UMLUTILSVER): archive/uml_utilities_$(UMLUTILSVER).tar.bz2
	mkdir -p $@
	tar -xj -C $@ -f $<

bin/uml_switch: uml_utilities_$(UMLUTILSVER)
	make -C uml_utilities_$(UMLUTILSVER)/tools
	mkdir -p bin
	cp uml_utilities_$(UMLUTILSVER)/tools/uml_router/uml_switch bin
	cp uml_utilities_$(UMLUTILSVER)/tools/port-helper/port-helper bin
	cp uml_utilities_$(UMLUTILSVER)/tools/tunctl/tunctl bin
	cp uml_utilities_$(UMLUTILSVER)/tools/mconsole/uml_mconsole bin

# This is the shortest path to an updated root
linux: linux-$(KVER)/linux
	rm -rf root
	$(MAKE) -f Makefile root
	touch linux-$(KVER)/linux
	$(MAKE) -f Makefile bin/linux

# Rebuild with correct kernel modules
bin/linux: linux-$(KVER)/linux
	INSTALL_MOD_PATH=$(TOP)/root make -C linux-$(KVER) ARCH=um modules_install
	rm root/lib/modules/$(KVER)/build root/lib/modules/$(KVER)/source
	./linux-$(KVER)/scripts/gen_initramfs_list.sh -u $(UID) -g $(GID)\
	  $(TOP)/root $(TOP)/root/dev/initramfs_list\
	  > linux-$(KVER)/initramfs_list
	rm -f linux-$(KVER)/usr/initramfs_data.cpio.gz
	make -C linux-$(KVER) ARCH=um
	touch linux-$(KVER)/linux
	cp linux-$(KVER)/linux $@
	strip $@

menuconfig: linux-$(KVER)/linux
	make -C linux-$(KVER) ARCH=um menuconfig
	make -C linux-$(KVER) ARCH=um
	touch linux-$(KVER)/linux

# This target builds an incomplete ramfs; kernel modules are missing
linux-$(KVER)/linux: config/linux-config linux-$(KVER) root
	mkdir -p bin
	cp -L config/linux-config linux-$(KVER)/.config
	chmod +x linux-$(KVER)/scripts/gen_initramfs_list.sh
	./linux-$(KVER)/scripts/gen_initramfs_list.sh -u $(UID) -g $(GID)\
	  $(TOP)/root $(TOP)/root/dev/initramfs_list\
	  > linux-$(KVER)/initramfs_list
	make -C linux-$(KVER) ARCH=um oldconfig
	make -C linux-$(KVER) ARCH=um
	touch $@

linux-$(KVER): archive/linux-$(KVER).tar.bz2
	tar xjf $<
	test -r config/linux-$(KVER).patch &&\
	   patch -d linux-$(KVER) -b -p1 < config/linux-$(KVER).patch
	touch $@

bin/busybox: config/busybox-config busybox-$(BBVER)
	mkdir -p bin
	cp -L config/busybox-config busybox-$(BBVER)/.config
	make -C busybox-$(BBVER) oldconfig
	make -C busybox-$(BBVER)
	cp busybox-$(BBVER)/busybox bin

root: bin/busybox root_template
	rm -rf root
	cp -rL root_template root
	cp -L bin/busybox root/bin
	for n in bin/sh sbin/init; do ln -fs /bin/busybox root/$$n; done
	cp -L `ldd ./bin/busybox | tr -s ' \t' '\n' | grep /lib | sed -e 's,/tls,,'` root/lib
	cp -L /lib/libnss_files.so.? /lib/libpthread.so.? root/lib
	chown -hR $(UID):$(GID) root

busybox-$(BBVER): archive/busybox-$(BBVER).tar.bz2
	tar xjf $<
	touch $@

archive: archive/linux-$(KVER).tar.bz2 archive/busybox-$(BBVER).tar.bz2 \
	archive/uml_utilities_$(UMLUTILSVER).tar.bz2

archive/linux-$(KVER).tar.bz2:
	mkdir -p archive
	wget -O $@ $(KURL)

archive/busybox-$(BBVER).tar.bz2:
	mkdir -p archive
	wget -O $@ $(BBURL)

archive/uml_utilities_$(UMLUTILSVER).tar.bz2:
	mkdir -p archive
	wget -O $@ $(UMLUTILSURL)

clean:
	rm -rf pax_global_header linux-$(KVER) linux busybox-$(BBVER) \
		bin/linux bin/busybox bin/port-helper bin/tunctl bin/uml_switch root \
		uml_utilities_$(UMLUTILSVER)

realclean: clean
	rm -rf archive
