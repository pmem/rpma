#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023, Intel Corporation
#

#
# install-rdma-core.sh - installs rdma-core libraries from sources with support for:
#                        native atomic write and native flush
#

set -ex

# rdma-core v45.0 with support for native atomic write and native flush
VERSION="45.0"

WORKDIR=$(pwd)

#
# Install rdma-core libraries from a release package
#
wget https://github.com/linux-rdma/rdma-core/archive/refs/tags/v${VERSION}.tar.gz
tar -xzf v${VERSION}.tar.gz
rm v${VERSION}.tar.gz

mv rdma-core-${VERSION} rdma-core
cd rdma-core
./build.sh
cd $WORKDIR
