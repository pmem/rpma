#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
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
	libibverbs-dev \
	librdmacm-dev \
	libunwind-dev \
	linux-modules-extra-$(uname -r) \
	pandoc"

export DEBIAN_FRONTEND=noninteractive

# Update existing packages
sudo apt-get update --allow-unauthenticated

# update list of sources
MIRROR="http://ddebs.ubuntu.com"
echo "
deb $MIRROR $(lsb_release -cs) main restricted universe multiverse
deb $MIRROR $(lsb_release -cs)-updates main restricted universe multiverse
deb $MIRROR $(lsb_release -cs)-proposed main restricted universe multiverse" | \
	sudo tee -a /etc/apt/sources.list.d/ddebs.list

# import missing GPG keys:
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C8CAB6595FDFF622
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 536F8F1DE80F6A35

# Update existing packages once again
sudo apt-get update --allow-unauthenticated

# Install new packages
sudo apt-get install --assume-yes --no-install-recommends --allow-unauthenticated \
	$BASE_DEPS \
	$EXAMPLES_DEPS \
	$TOOLS_DEPS \
	$TESTS_DEPS \
	$RPMA_DEPS
