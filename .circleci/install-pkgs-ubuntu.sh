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
	libibverbs-dev \
	librdmacm-dev \
	libunwind-dev \
	linux-modules-extra-$(uname -r) \
	pandoc \
	rdma-core"

# Install all required packages
sudo apt-get update
sudo apt-get install --assume-yes --no-install-recommends \
	$BASE_DEPS \
	$EXAMPLES_DEPS \
	$RPMA_DEPS
