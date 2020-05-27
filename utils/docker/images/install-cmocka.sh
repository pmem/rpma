#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# install-cmocka.sh - installs cmocka.org
#

set -e

git clone https://git.cryptomilk.org/projects/cmocka.git
mkdir cmocka/build
cd cmocka/build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
sudo make -j$(nproc) install
cd ../..
rm -r cmocka
