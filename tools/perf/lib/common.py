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
    return {'input_file': string, 'json': data}

def uniq(elems):
    """Generate a set of unique objects"""
    output = {}
    id = 0
    for elem in elems:
        duplicate = False
        # look up for duplicates
        for oid, other in output.items():
            if elem == other:
                # creating a relationship
                elem.set_id(oid)
                duplicate = True
                break
        # a new benchmark found
        if not duplicate:
            elem.set_id(id)
            output[id] = elem
            id += 1
    return output
