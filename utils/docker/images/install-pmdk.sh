#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

#
# install-pmdk.sh <package_type> - installs PMDK
#

set -e

if [ "${SKIP_PMDK_BUILD}" ]; then
	echo "Variable 'SKIP_PMDK_BUILD' is set; skipping building PMDK"
	exit
fi

PACKAGE_TYPE=$1
PREFIX=${2:-/usr}

# v1.8, 31.01.2020
PMDK_VERSION="1.8"

git clone https://github.com/pmem/pmdk
cd pmdk
git checkout $PMDK_VERSION

if [ "$PACKAGE_TYPE" = "" ]; then
	make -j$(nproc) PMEM2_INSTALL=y install prefix=$PREFIX
else
	make -j$(nproc) PMEM2_INSTALL=y BUILD_PACKAGE_CHECK=n $PACKAGE_TYPE
	if [ "$PACKAGE_TYPE" = "dpkg" ]; then
		sudo dpkg -i dpkg/libpmem2_*.deb dpkg/libpmem2-dev_*.deb
	elif [ "$PACKAGE_TYPE" = "rpm" ]; then
		sudo rpm -i rpm/*/pmdk-debuginfo-*.rpm
		sudo rpm -i rpm/*/libpmem2-*.rpm
	fi
fi

cd ..
rm -r pmdk
