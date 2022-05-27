#!/bin/bash -e
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

PKG=$1
git clone https://aur.archlinux.org/$PKG.git
chown -R user:user ./$PKG
cd $PKG
sudo -u user makepkg -si --noconfirm --skippgpcheck
cd ..
rm -r $PKG
