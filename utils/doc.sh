#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation

DIR=$1
if [ "$DIR" == "" ]; then
	echo "Error: missing directory parameter"
	echo
	echo "Usage: $0 <directory-to-search>"
	exit 1
fi

find $DIR -name '*.h' -print0 | while read -d $'\0' man
do
	errors=$(src2man -r RPMA -v "RPMA Programmer's Manual" ${man} 2>&1 | grep -e 'awk: cmd')
	if [ "$errors" != "" ]; then
		echo
		echo "src2man: errors found in the \"$man\" file:"
		echo "$errors"
		echo
	fi

	files=$(src2man -r RPMA -v "RPMA Programmer's Manual" ${man} 2>&1 | grep -v -e 'awk: cmd')
	for f in $files; do
		# get rid of a FILE section (last two lines of the file)
		mv $f $f.tmp
		head -n -2 $f.tmp > $f
		rm $f.tmp

		cat $f | groff -mandoc -Thtml >$f.html
	done
done
