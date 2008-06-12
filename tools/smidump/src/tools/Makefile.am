#
# Makefile.am --
#
#      Template to generate the tools Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am,v 1.33 2002/06/08 12:58:26 schoenw Exp $
#

INCLUDES		= -I../lib

EXTRA_DIST		= smidump.h shhopt.h \
			  $(man_MANS) \
			  dump-fig.c

bin_PROGRAMS		= smiquery smilint smidump smidiff

bin_SCRIPTS		= smistrip smicache

man_MANS		= smiquery.1 smilint.1 smidump.1 smidiff.1 \
			  smistrip.1 smicache.1

smiquery_SOURCES	= smiquery.c shhopt.c
smiquery_LDADD		= ../lib/libsmi.la

smilint_SOURCES		= smilint.c shhopt.c
smilint_LDADD		= ../lib/libsmi.la

smidump_SOURCES		= smidump.c smidump.h shhopt.c \
			  dump-sming.c dump-smi.c dump-smiv3.c \
			  dump-imports.c dump-types.c dump-tree.c \
			  dump-mosy.c dump-corba.c dump-netsnmp.c \
			  dump-jax.c dump-xml.c dump-identifiers.c \
			  dump-metrics.c dump-cm.c dump-python.c \
			  dump-perl.c dump-scli.c dump-xsd.c dump-sppi.c
#			  dump-fig.c
smidump_LDADD		= ../lib/libsmi.la

smidiff_SOURCES		= smidiff.c shhopt.c
smidiff_LDADD		= ../lib/libsmi.la
