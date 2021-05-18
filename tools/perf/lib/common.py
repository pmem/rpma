#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# common.py -- common definitions (EXPERIMENTAL)
#

import json
import os.path

def dir_path(string):
    if os.path.exists(string):
        if not os.path.isdir(string):
            raise NotADirectoryError(string)
    else:
        os.mkdir(string)
    return os.path.abspath(string)

def json_from_file(string):
    if not os.path.isfile(string):
        raise FileNotFoundError(string)
    with open(string, "r") as read_file:
        data = json.load(read_file)
    return data
