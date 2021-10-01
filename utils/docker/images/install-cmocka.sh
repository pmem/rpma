#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# install-cmocka.sh - installs cmocka.org
#

# cmocka-1.1.5-26-g672c5ce - pull latest fixes
CMOCKA_VERSION=672c5cee79eb412025c3dd8b034e611c1f119055

git clone https://git.cryptomilk.org/projects/cmocka.git
if [ $? -ne 0 ]; then
	# in case of a failure retry after updating certificates
	set -e
	openssl s_client -showcerts -servername git.cryptomilk.org -connect git.cryptomilk.org:443 </dev/null 2>/dev/null \
		| sed -n -e '/BEGIN\ CERTIFICATE/,/END\ CERTIFICATE/ p'  > git-cryptomilk.org.pem
	cat git-cryptomilk.org.pem | sudo tee -a /etc/ssl/certs/ca-certificates.crt
	rm git-cryptomilk.org.pem
	git clone https://git.cryptomilk.org/projects/cmocka.git
fi

set -e

mkdir cmocka/build
cd cmocka/build

git checkout $CMOCKA_VERSION

cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
sudo make -j$(nproc) install
cd ../..
sudo rm -rf cmocka
