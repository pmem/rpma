#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation

#
# Finds applicable area name for specified commit id.
#

if [ -z "$1" ]; then
	echo "Missing commit id argument."
	exit 1
fi

files=$(git log $1 -1 --format=oneline --name-only | grep -v -e "$1")

git show -q $1 | cat

echo
echo "Modified files:"
echo "$files"

function categorize() {
	category=$1
	shift
	cat_files=`echo "$files" | grep $*`

	if [ -n "${cat_files}" ]; then
		echo "$category"
		files=`echo "$files" | grep -v $*`
	fi
}

echo
echo "Areas computed basing on the list of modified files: (see utils/check-area.sh for full algorithm)"

categorize rpma      -e "^src/"
categorize test      -e "^tests/"
categorize examples  -e "^examples/"
categorize doc       -e "^doc/" -e ".md\$" -e "^ChangeLog" -e "README"
categorize common    -e "^utils/" \
			-e ".inc\$" \
			-e ".yml\$" \
			-e ".gitattributes" \
			-e ".gitignore" \
			-e "^.mailmap\$" \
			-e "Makefile\$"

echo
echo "If the above list contains more than 1 entry, please consider splitting"
echo "your change into more commits, unless those changes don't make sense "
echo "individually (they do not build, tests do not pass, etc)."
echo "For example, it's perfectly fine to use 'obj' prefix for one commit that"
echo "changes libpmemobj source code, its tests and documentation."

if [ -n "$files" ]; then
	echo
	echo "Uncategorized files:"
	echo "$files"
fi
