#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2023, Intel Corporation
#

#
# install-pmdk.sh - installs PMDK libraries
#

# PMDK version: 8074b19b1d9b40bcaaed7e0dc0622dccf8007f2f (1.12.1-119-g8074b19b1)
# with required fixes, see:
# https://github.com/pmem/pmdk/issues/5540
PMDK_VERSION=8074b19b1d9b40bcaaed7e0dc0622dccf8007f2f

WORKDIR=$(pwd)

#
# Install PMDK libraries from a release package with already generated documentation.
#
wget https://github.com/pmem/pmdk/archive/${PMDK_VERSION}.zip
gzip -d pmdk-${PMDK_VERSION}.tar.gz
cd pmdk-${PMDK_VERSION}
make -j$(nproc) NDCTL_ENABLE=n
sudo make -j$(nproc) install prefix=/usr NDCTL_ENABLE=n
cd $WORKDIR
rm -rf pmdk-${PMDK_VERSION}
