#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# patch-txt2man.sh - patch txt2man to fix the following bug:
#                    https://github.com/mvertes/txt2man/issues/21
#

set -e

FILE=$(which txt2man)

if [ "$FILE" == "" ]; then
	echo "Error: txt2man not found"
	exit 1
fi

echo "Patching $FILE ..."
cp $FILE .
# apply the patch: https://github.com/mvertes/txt2man/pull/18
patch < ./txt2man.patch
sudo mv ./txt2man $FILE
