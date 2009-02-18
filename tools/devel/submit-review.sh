#!/bin/sh
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
# Author(s): Wind River Systems
#

HG=`which hg`

if [ -z $EDITOR ]; then
   EDITOR="vi"
fi

usage="Usage: submit-review.sh [-q|-c] [-r rev] [-b url] [-d dest] [-s subject]"

while getopts ":qcr:b:d:s:" opt; do
   case $opt in
      q ) mq=1 ;;
      c ) cs=1 ;;
      r ) rev=$OPTARG ;;
      b ) branch=$OPTARG ;;
      d ) dest=$OPTARG ;;
      s ) subject=$OPTARG ;;
      \?) echo $usage
          exit 1 ;;
   esac
done

shift $(($OPTIND -1))

if [ $mq ] && [ $cs ]; then
   echo "$0: [-q|-c] are mutually exclusive options"
   echo $usage
   exit 1
elif [ ! $mq ] && [ ! $cs ]; then
   cs=1
fi

if [ -z $dest ]; then
   rr=`mktemp -d`
   if [ $? != 0 ]; then
      echo "$0: mktemp failed"
      exit 1
   fi
else
   rr=$dest
   if [ ! -d $rr ]; then
      mkdir $rr
      if [ $? != 0 ]; then
         echo "$0: mkdir $rr failed"
         exit 1
      fi
   fi
fi

if [ -z $subject ]; then
   subject="Review Request"
fi

if [ $cs ]; then
   if [ -z $rev ]; then
      rev="tip"
   fi
   echo "Exporting changeset(s) '$rev' for review"
elif [ $mq ]; then
   echo "Exporting the patch queue for review"
fi

echo "The review package will be placed under $rr"

hgroot=`$HG root`

if [ $mq ]; then
   if [ ! -d $hgroot/.hg/patches/ ]; then
      echo "$0: Did you init the patch queue properly?"
      exit 1
   else
      cp $hgroot/.hg/patches/*.patch $rr
      cp $hgroot/.hg/patches/series $rr
   fi
elif [ $cs ]; then
   $HG export -g -o $rr/%R.patch $rev
   cd $rr
   ls -1 *.patch >> series
   cd -
fi

cat <<ETX >> $rr/rr
Summary: <<FILL_ME>>
Review request for Trac Ticket(s): <<IF_ANY_LIST_THE_#>>
Peer Reviewer(s): <<FILL_ME>>
Maintainer: <<FILL_ME>>
ETX

if [ ! -z $branch ]; then
   echo "Development branch: $branch" >> $rr/rr
fi

cat <<ETX >> $rr/rr
Affected branch(es): <<LIST_AFFECTED_BRANCH(ES)>>

--------------------------------
Impacted area       Impact y/n
--------------------------------
 Docs                    n
 Tests                   n
 Build system            n
 RPM/packaging           n
 Configuration files     n
 Startup scripts         n
 Samples                 n
 SAF services            n
 non-SAF services        n
 Other                   n


Comments (indicate scope for each "y" above):
---------------------------------------------
ETX
series=`cat $rr/series`
for l in $series; do
   echo " $l:"
   echo ""
   echo "  <<PATCH_COMMENTS_HERE>>"
   echo ""
done >> $rr/rr
cat <<ETX >> $rr/rr

Added Files:
------------
ETX
new=`egrep -A 2 -s '^new file mode ' $rr/*.patch | grep -s '^+++ ' | awk '{ print $2 }' | sort -u`
for l in $new; do
   echo " $l"
done >> $rr/rr

cat <<ETX >> $rr/rr


Removed Files:
--------------
ETX
del=`egrep -A 1 -s '^deleted file mode ' $rr/*.patch | grep -s '^--- ' | awk '{ print $2 }' | sort -u`
for l in $del; do
   echo " $l"
done >> $rr/rr

cat <<ETX >> $rr/rr


Remaining Changes (diffstat):
-----------------------------
ETX
if [ $mq ]; then
   $HG qdiff | diffstat >> $rr/rr
elif [ $cs ]; then
   $HG export -g $rev | diffstat >> $rr/rr
fi
cat <<ETX >> $rr/rr


Testing Applicable to:
----------------------
 <<FILL_ME>>


Testing Commands:
-----------------
 <<FILL_ME>>


Testing, Expected Results:
--------------------------
 <<FILL_ME>>


Conditions of Submission:
-------------------------
 <<FILL_ME>>


Arch    Built      Started     Linux distro
-------------------------------------------
MIPS      n         n
MIPS64    n         n
x86       n         n
x86_64    n         n
PPC       n         n
PPC64     n         n
SPARC     n         n
SPARC64   n         n
ETX

$EDITOR $rr/rr

if [ $mq ]; then
   $HG email -s "$subject" --intro --desc $rr/rr --cc devel@list.opensaf.org qbase:qtip
elif [ $cs ]; then
   $HG email -s "$subject" --intro --desc $rr/rr --cc devel@list.opensaf.org $rev
fi
