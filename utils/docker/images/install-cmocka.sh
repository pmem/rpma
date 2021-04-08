#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# install-cmocka.sh - installs cmocka.org
#

set -e

git clone https://git.cryptomilk.org/projects/cmocka.git
mkdir cmocka/build
cd cmocka/build

# cmocka-1.1.5-26-g672c5ce - pull latest fixes
git checkout 672c5cee79eb412025c3dd8b034e611c1f119055

cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
sudo make -j$(nproc) install
cd ../..
sudo rm -rf cmocka
