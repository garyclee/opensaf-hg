#
# Makefile.am --
#
#      Template to generate the doc Makefile.in using automake.
#
# Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
#
# See the file "COPYING" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id: Makefile.am,v 1.5 2000/02/16 21:14:51 strauss Exp $
#

EXTRA_DIST		= draft-irtf-nmrg-sming-02.txt

smi.gif: smi.eps
	epsffit -sc 0 0 600 600 smi.eps smi.eps.tmp
	ps2gif smi.eps.tmp smi.gif.tmp
	giftopnm smi.gif.tmp | pnmcrop | ppmtogif > smi.gif
	rm smi.eps.tmp smi.gif.tmp
