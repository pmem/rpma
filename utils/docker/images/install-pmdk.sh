#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

#
# install-pmdk.sh - installs PMDK libraries
#

# PMDK version: 1.12.0
PMDK_VERSION=1.12.0

WORKDIR=$(pwd)

#
# Install PMDK libraries from a release package with already generated documentation.
#
wget https://github.com/pmem/pmdk/releases/download/${PMDK_VERSION}/pmdk-${PMDK_VERSION}.tar.gz
tar -xzf pmdk-${PMDK_VERSION}.tar.gz
cd pmdk-${PMDK_VERSION}
make -j$(nproc) NDCTL_ENABLE=n
sudo make -j$(nproc) install prefix=/usr NDCTL_ENABLE=n
cd $WORKDIR
rm -rf pmdk-${PMDK_VERSION}
