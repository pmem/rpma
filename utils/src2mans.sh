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
mkdir -p md

find $DIR -name '*.h' -print0 | while read -d $'\0' MAN
do
	MANUALS="$(mktemp)"
	ERRORS="$(mktemp)"

	src2man -r RPMA -v "RPMA Programmer's Manual" $MAN > $MANUALS 2> $ERRORS
	# gawk 5.0.1 does not recognize expressions \;|\,|\o  as regex operator
	sed -i -r "/warning: regexp escape sequence \`[\][;,o]' is not a known regexp operator/d" $ERRORS
	# remove empty lines
	sed -i '/^$/d' $ERRORS

	if [[ -s "$ERRORS" ]]; then
		echo "src2man: errors found in the \"$MAN\" file:"
		cat $ERRORS
	fi

	for f in $(cat $MANUALS | xargs); do
		# get rid of a FILE section (last two lines of the file)
		mv $f $f.tmp
		head -n -2 $f.tmp > $f
		rm $f.tmp
		cat $f | groff -mandoc -Thtml > html/$f.html
		
		# generate a md file
		cat mans_header.md > md/$f.md
		mandoc -mdoc -T markdown $f >> md/$f.md
	done
	rm $MANUALS $ERRORS
done
