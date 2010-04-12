#!/bin/sh
#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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

# This script can be used as a Mercurial hook the enforce commit message policy
# Usage: ./hg-check-desc-hook.sh <changeset>
#
# The expected format of the full Mercurial {desc} is:
#
# module: short description of the change, maximum 72 chars line
# <blank line>
# Ticket ID: #500, #501
# <blank line>
# Begining of the the long description, like with the short description,
# the long description should not have lines longer than 72 chars and it
# should at least be 2 lines long.
# <blank line>
# Signed-off-by: Full Name <full.name@email.com> (Mandatory)
# Reviewed-by: Full Name <full.name@email.com> (Optional)

HG_NODE=$1
MAX_DESC_LINE_LEN=72

# The tag and merge commits should not have any restrictions other than
# the module naming convention to detect those exceptions
check_commit_exception() {
	echo "$1" | grep -q -e "^tag: .*$" -e "^merge: .*$"
	rc=$?

	if [ $rc -eq 0 ]; then
		# We've got a 'tag: ...' or 'merge: ..." commit
		# this is a policy exception
		exit 0
	fi
}

check_short_desc() {
	# The patch header first line contains the short description
	short_desc=`echo "$1" | sed -n '1p'`
	short_desc_len=`expr length "$short_desc"`

	match=`expr match "$short_desc" "^[a-z]*: .*$"`
	if [ $match -eq 0 ]; then
		echo "changeset $2: The short description does not begin with the 'module: ' syntax" >&2
		exit 1
	fi

	check_commit_exception "$short_desc"

	if [ $short_desc_len -gt $MAX_DESC_LINE_LEN ]; then
		echo "changeset $2: The short description is longer than 72 chars" >&2
		exit 1
	fi
}

check_ticket_id() {
	# The patch header Trac ticket(s) list is on line 3, line 2 is always left blank
	tickets=`echo "$1" | sed -n '3p'`

	if [ -z "$tickets" ]; then
		echo "changeset $2: The changeset is not listing any Trac ticket(s)" >&2
		exit 1
	fi

	echo "$tickets" | egrep -q "^Ticket ID: (#[0-9]*)([, ]{1,}#[0-9]*)?$"
	rc=$?

	if [ $rc -ne 0 ]; then
		echo "changeset $2: The Trac ticket(s) listing is not formatted properly" >&2
		exit 1
	fi
}

check_long_desc() {
	# The patch header long description starts at line 5, line 4 is always left blank
	long_desc_line1=`echo "$1" | sed -n '5p'`
	long_desc_line2=`echo "$1" | sed -n '6p'`

	if [ -z "$long_desc_line1" ] || [ -z "$long_desc_line2" ]; then
		echo "changeset $2: The long description should be at least 2 lines long" >&2
		exit 1
	fi

	# Make sure we are skipping Signed-off-by and Reviewed-by lines
	echo "$1" | while read line; do
		match=`expr match "$line" ".*-by: "`
		if [ $match -eq 0 ]; then
			desc_line_len=`expr length "$line"`
			if [ $desc_line_len -gt $MAX_DESC_LINE_LEN ]; then
				echo "changeset $2: One of the long description line is longer than 72 chars" >&2
				exit 1
			fi
		fi
	done
}

check_signed_off() {
	# The patch header last line should contain a Signed-off-by tag
	last_line=`echo "$1" | sed -n '$p'`

	echo "$1" | grep -q "Signed-off-by: "
	rc=$?

	if [ $rc -ne 0 ]; then
		echo "changeset $2: The patch does not contain any Signed-off-by tag(s)" >&2
		exit 1
	fi

	echo "$last_line" | grep -q -e "Signed-off-by: " -e "Reviewed-by: "
	rc=$?

	if [ $rc -ne 0 ]; then
		echo "changeset $2: The last line of the message should be a Signed-off-by or Reviewed-by tag" >&2
		exit 1
	fi
}

# Get a list of changesets in this changegroup
for rev in `hg log --template '{rev}\n' -r $HG_NODE`; do
	desc=`hg log --template '{desc}\n' -r $rev`
	check_short_desc "$desc" "$rev"
	check_ticket_id "$desc" "$rev"
	check_long_desc "$desc" "$rev"
	check_signed_off "$desc" "$rev"
done

exit 0
