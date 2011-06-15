############################################################################
#
# (C) Copyright 2011 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Wind River Systems, Inc.
#
############################################################################

# Sample Makefile to build the Python package outside of the OpenSAF build
# make -f Makefile.mk
# make -f Makefile.mk dist
# etc.

PYTHON=python
PREFIX=/usr/local
PACKAGE_NAME=pyosaf

all: build

build:
	$(PYTHON) setup.py build

clean:
	-$(PYTHON) setup.py clean --all
	find . -name '*.py[cdo]' -exec rm -f '{}' ';'

distclean: clean
	rm -rf ${PACKAGE_NAME}.egg-info/ dist/

install: install-bin

install-bin: build
	$(PYTHON) setup.py install --root="$(DESTDIR)/" --prefix="$(PREFIX)" --force

dist:
	$(PYTHON) setup.py sdist
