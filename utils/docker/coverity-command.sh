#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

#
# coverity-command.sh - make script for the Coverity build
#

set -ex

pwd
mkdir -p build
cd build
cmake ..
make -j$1
