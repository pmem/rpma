#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation

mkdir ../build/doc/man3
mkdir ../build/doc/man7
mkdir ../build/doc/html

find . ../src/ -name '*.h' -print0 | while read -d $'\0' man
do
	files=$(src2man -r RPMA -v "RPMA Programmer's Manual" ${man} 2>&1)

	# get rid of a FILE section (last two lines of the file)
	for f in $files; do
		mv $f $f.tmp
		head -n -2 $f.tmp > $f
		rm $f.tmp
		cat $f | groff -mandoc -Thtml >$f.html
		if [[ $f == *.3 ]]; then
			mv $f ../build/doc/man3/
			mv $f.html ../build/doc/html/
		elif [[ $f == *.7 ]]; then
			mv $f ../build/doc/man7/
			mv $f.html ../build/doc/html/
		fi
	done
done
