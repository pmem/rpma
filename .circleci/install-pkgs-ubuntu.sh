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

TOOLS_DEPS="\
	python3-jinja2"

TESTS_DEPS="\
	pylint"

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

# Enable repositories with debug symbols packages (-dbgsym)
sudo apt-get install --assume-yes --no-install-recommends lsb-release ubuntu-dbgsym-keyring
echo "deb http://ddebs.ubuntu.com $(lsb_release -cs) main restricted universe multiverse
deb http://ddebs.ubuntu.com $(lsb_release -cs)-updates main restricted universe multiverse
deb http://ddebs.ubuntu.com $(lsb_release -cs)-proposed main restricted universe multiverse" | \
	sudo tee -a /etc/apt/sources.list.d/ddebs.list
sudo apt-get update

sudo apt-get install --assume-yes --no-install-recommends \
	$BASE_DEPS \
	$EXAMPLES_DEPS \
	$TOOLS_DEPS \
	$TESTS_DEPS \
	$RPMA_DEPS
