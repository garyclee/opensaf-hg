#!/usr/bin/perl5
#*******************************************************************************
#       $Header:$
# Globals: This package creates global variables (including DRYRUN and VERBOSE)
#          to share between library modules and the main calling routine without
#          having to explicitly pass values between functions
#
#
#      -*- OpenSAF  -*-
#
# (C) Copyright 2008 The OpenSAF Foundation
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
# Author(s):
#           Hewlett-Packard Company
#
#*******************************************************************************
package Globals;

use strict 'vars';

sub import {
    my $from = caller(1);
    my $to   = caller();

    *{"$to\::DRYRUN"} = \${"$from\::DRYRUN"};
    *{"$to\::GLOBAL_H"} = \${"$from\::GLOBAL_H"};
    *{"$to\::VERBOSE"} = \${"$from\::VERBOSE"};
    *{"$to\::ScriptName"} = \${"$from\::ScriptName"};
}
1;
