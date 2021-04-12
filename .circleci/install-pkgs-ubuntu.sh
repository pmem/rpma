#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# .circleci/install-pkgs-ubuntu.sh - install packages for the CircleCI Ubuntu image
#

set -e

BASE_DEPS="\
	apt-utils \
	build-essential \
	devscripts \
	git \
	pkg-config \
	sudo \
	whois"

EXAMPLES_DEPS="\
	libpmem-dev \
	libprotobuf-c-dev \
	valgrind"

RPMA_DEPS="\
	cmake \
	curl \
	gawk \
	groff \
	graphviz \
	ibverbs-providers-dbgsym \
	libibverbs-dev \
	libibverbs1-dbgsym \
	librdmacm-dev \
	librdmacm1-dbgsym \
	libunwind-dev \
	linux-modules-extra-$(uname -r) \
	pandoc"

# Update existing packages
sudo apt-get update

# Install lsb-release (will be needed by /etc/apt/sources.list.d/ddebs.list)
sudo apt-get install --assume-yes --no-install-recommends lsb-release

# Enable repositories with debug symbols packages (-dbgsym)
sudo apt-get install ubuntu-dbgsym-keyring
sudo cp .circleci/ddebs.list /etc/apt/sources.list.d/ddebs.list
sudo apt-get update

sudo apt-get install --assume-yes --no-install-recommends \
	$BASE_DEPS \
	$EXAMPLES_DEPS \
	$RPMA_DEPS
