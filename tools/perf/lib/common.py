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

ENCODE = json.JSONEncoder(indent=4).encode

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
    with open(string, 'r', encoding='utf-8') as read_file:
        data = json.load(read_file)
    # return the content of the file
    return {'input_file': string, 'json': data}

def get_benchmark_result_path(result_dir: str, identifier: int) -> str:
    """a path to the file with all the collected results

    Args:
        result_dir: a directory where the intermediate and final products
          of the benchmarking process will be stored.

        identifier: an identifier of `lib.benchmark.base.Benchmark` you
          are looking for.

    Returns:
        The path to the file with all the collected results.
    """
    return os.path.join(result_dir, 'benchmark_{}.json'.format(str(identifier)))

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
                elem.identifier = oid
                duplicate = True
                break
        # a new benchmark found
        if not duplicate:
            elem.identifier = id
            output[id] = elem
            id += 1
    return output

def escape(x):
    """Escape markdown special characters"""
    if isinstance(x, str):
        return x.replace('_', '\\_')
    elif isinstance(x, list):
        return [escape(el) for el in x]
    else:
        raise ValueError('Unsupported type in escape(): ' + type(x))
