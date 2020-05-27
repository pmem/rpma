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

mkdir -p html

find $DIR -name '*.h' -print0 | while read -d $'\0' man
do
	errors="$(mktemp)"
	output="$(mktemp)"
	files="$(mktemp)"

	src2man -r RPMA -v "RPMA Programmer's Manual" ${man} > $output 2>&1
	# gawk 5.0.1 does not recognize expressions \;|\,|\o  as regex operator
	sed -i -r "/warning: regexp escape sequence \`[\][;,o]' is not a known regexp operator/d" $output
	# remove empty lines
	sed -i '/^$/d' $output

	cat $output | while read line
	do
		if [[ -f "${line}" ]]; then
			echo ${line} >> $files
		else
			echo ${line} >> $errors
		fi
	done

	if [[ -s "$errors" ]]; then
		echo "src2man: errors found in the \"$man\" file:"
		cat $errors
		exit 1
	fi

	for f in $(cat $files | xargs); do
		# get rid of a FILE section (last two lines of the file)
		mv $f $f.tmp
		head -n -2 $f.tmp > $f
		rm $f.tmp
		cat $f | groff -mandoc -Thtml > html/$f.html
	done
	rm $errors $output $files
done
