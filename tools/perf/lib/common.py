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
    """Validate provided directory path"""
    if os.path.exists(string):
        if not os.path.isdir(string):
            raise NotADirectoryError(string)
    else:
        # create if it does not exist
        os.mkdir(string)
    # make the path absolute before giving it back
    return os.path.abspath(string)

def json_from_file(string):
    """Validate provided JSON file"""
    if not os.path.isfile(string):
        raise FileNotFoundError(string)
    # read the file
    with open(string, "r") as read_file:
        data = json.load(read_file)
    # return the content of the file
    return data
