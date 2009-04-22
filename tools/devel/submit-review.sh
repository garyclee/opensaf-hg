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

dryrun=0; mq=0; cs=0; og=0;

usage="Usage: submit-review.sh [-t] [-q|-c|-o] [-r rev] [-d dest]"

while getopts ":tqcor:d:" opt; do
        case $opt in
                t ) dryrun=1 ;;
                q ) mq=1 ;;
                c ) cs=1 ;;
                o ) og=1 ;;
                r ) rev=$OPTARG ;;
                d ) dest=$OPTARG ;;
                \?) echo $usage
                    exit 1 ;;
        esac
done

shift $(($OPTIND -1))

if [ $mq -eq 0 ] && [ $cs -eq 0 ] && [ $og -eq 0 ]; then
        cs=1
fi

if [ -z "$dest" ]; then
        rr=`mktemp -d`
        if [ $? != 0 ]; then
                echo "$0: mktemp failed"
                exit 1
        fi
else
        rr=$dest
        if [ ! -d "$rr" ]; then
                mkdir $rr
                if [ $? != 0 ]; then
                        echo "$0: mkdir $rr failed"
                        exit 1
                fi
        fi
fi

if [ $dryrun -eq 1 ]; then
        echo "WARNING: Running in dry-run mode, nothing will be emailed"
fi

if [ $og -eq 1 ]; then
        if hg outgoing > /dev/null 2>&1; then
                echo "Exporting all outgoing changes for review"
        else
                echo "No changes found, nothing to review!"
                rm -rf $rr
                exit 1
        fi
elif [ $cs -eq 1 ]; then
        if [ -z "$rev" ]; then
                rev="tip"
        fi
        echo "Exporting changeset(s) '$rev' for review"
elif [ $mq -eq 1 ]; then
        qseries=`hg qseries`
        qapplied=`hg qapplied`
        if [ -z "$qseries" ]; then
                echo "Patch queue empty, nothing to review!"
                rm -rf $rr
                exit 1
        fi

        if [ "$qseries" != "$qapplied" ]; then
                echo "Patch series needs to be fully applied before review!"
                rm -rf $rr
                exit 1
        fi

        echo "Exporting the patch queue for review"
fi

echo "The review package will be placed under $rr"

hgroot=`$HG root`

if [ $og -eq 1 ]; then
        $HG outgoing -M -p > $rr/outgoing.patch
        cd $rr
        ls -1 *.patch >> series
        cd - > /dev/null 2>&1
elif [ $mq -eq 1 ]; then
        if [ ! -d "$hgroot/.hg/patches" ]; then
                echo "Did you qinit the patch queue properly?"
                rm -rf $rr
                exit 1
        else
                cp $hgroot/.hg/patches/*.patch $rr
                cp $hgroot/.hg/patches/series $rr
        fi
elif [ $cs -eq 1 ]; then
        $HG export -g -o $rr/%R.patch $rev
        cd $rr
        ls -1 *.patch >> series
        cd - > /dev/null 2>&1
fi

cat <<ETX >> $rr/rr
Summary: <<FILL_ME>>
Review request for Trac Ticket(s): <<IF_ANY_LIST_THE_#>>
Peer Reviewer(s): <<FILL_ME>>
Maintainer: <<FILL_ME>>
Development branch: <<IF_ANY_GIVE_THE_URL>>
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
if [ $og -eq 1 ]; then
        $HG outgoing -M -p | diffstat >> $rr/rr
else
        if [ $mq -eq 1 ]; then
                $HG diff -r $(hg parents -r qbase --template '#rev#') -r qtip | diffstat >> $rr/rr
        elif [ $cs -eq 1 ]; then
                $HG export -g $rev | diffstat >> $rr/rr
        fi
fi
cat <<ETX >> $rr/rr


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

while [ -z "$subject" ]; do
        read -p "Subject: Review Request for " -e subject
done

while [ -z "$toline" ]; do
        read -p "To: " -e toline
done

COMMAND="$HG email -s 'Review Request for $subject' --intro --desc $rr/rr --to '$toline' --cc devel@list.opensaf.org"

if [ $dryrun -eq 1 ]; then
        COMMAND+=" -n"
fi

if [ $og -eq 1 ]; then
        COMMAND+=" -o"
elif [ $mq -eq 1 ]; then
        COMMAND+=" qbase:qtip"
elif [ $cs -eq 1 ]; then
        COMMAND+=" $rev"
fi

if [ $dryrun -eq 1 ]; then
        echo "Email will be dumped as $rr/review.mail"
        eval $COMMAND > $rr/review.mail
else
        eval $COMMAND
fi

