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
	set -x
	echo WORKDIR=$WORKDIR
	sudo_password chown -R $(id -u).$(id -g) $WORKDIR
	git config --global --add safe.directory "$WORKDIR"
	sudo_password git config --global --add safe.directory "$WORKDIR"
fi
