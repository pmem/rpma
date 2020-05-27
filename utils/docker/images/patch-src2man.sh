#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# patch-src2man.sh - patch src2man to fix the following bug:
#                    https://github.com/mvertes/txt2man/issues/23
#

set -e

FILE=$(which src2man)

if [ "$FILE" == "" ]; then
	echo "Error: src2man not found"
	exit 1
fi

echo "Patching $FILE ..."
sudo sed -i 's/print title "." sect > "\/dev\/stderr"/print title "." sect > "\/dev\/stdout"/g' $FILE
