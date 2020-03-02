#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation

#
# src2mans -- extract man pages from source files
#

DIR=$1
if [ "$DIR" == "" ]; then
	echo "Error: missing directory parameter"
	echo
	echo "Usage: $0 <directory-to-search>"
	exit 1
fi

find $DIR -name '*.h' -print0 | while read -d $'\0' man
do
	errors=""
	files=""
	src2man -r RPMA -v "RPMA Programmer's Manual" ${man} > output.txt 2>&1
	sed -i -r "/warning: regexp escape sequence \`(\\\)?[;,o]' is not a known regexp operator/d" output.txt

	cat output.txt | while read line
	do
		if [[ -f "${line}" ]]; then
			files+="${line} "
		else
			errors+="${line}\n"
		fi
	done
	rm output.txt

	if [ "${errors}" != "" ]; then
		echo "src2man: errors found in the \"$man\" file:"
		echo -e "$errors"
	fi

	for f in $files; do
		# get rid of a FILE section (last two lines of the file)
		mv $f $f.tmp
		head -n -2 $f.tmp > $f
		rm $f.tmp

		cat $f | groff -mandoc -Thtml >$f.html
	done
done
