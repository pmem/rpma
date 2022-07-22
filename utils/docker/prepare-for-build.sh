#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

#
# prepare-for-build.sh - prepare the Docker image for the build
#

set -e

function sudo_password() {
	echo $USERPASS | sudo -Sk $*
}

# this should be run only on CIs
if [ "$CI_RUN" == "YES" ]; then
	echo WORKDIR=$WORKDIR
	sudo_password chown -R $(id -u):$(id -g) $WORKDIR
fi

# fix for: https://github.com/actions/checkout/issues/766 (git CVE-2022-24765)
git config --global --add safe.directory "$WORKDIR"
sudo_password git config --global --add safe.directory "$WORKDIR"
