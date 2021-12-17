#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# common.py
#

"""common tools (EXPERIMENTAL)"""

import json
import os.path

from typing import Union

#: a JSON encoder with nice to read indent
ENCODE = json.JSONEncoder(indent=4).encode

def dir_path(string: str) -> str:
    """validate path and return it as an absolute path

    **Note**: If `string` does not exist in the filesystem it is created
    (`os.mkdir()`).

    Args:
        string: a path to process

    Returns
        An absolute path made from `string`.

    Raises:
        NotADirectoryError: If the `string` exists but it is not a directory.
    """
    if os.path.exists(string):
        if not os.path.isdir(string):
            raise NotADirectoryError(string)
    else:
        # create if it does not exist
        os.mkdir(string)
    # make the path absolute before giving it back
    return os.path.abspath(string)

def json_from_file(string: str) -> dict:
    """load a `string` JSON file

    Args:
        string: a path to a JSON file

    Return:
        An `output` dictionary composed of:

        - `output['input_file']` is the path of the JSON file
        - `output['json']` is the contents of the JSON file as a Python object
          (`json.load()`).

    Raises:
        FileNotFoundError: If the `string` file does not exist.
    """
    if not os.path.isfile(string):
        raise FileNotFoundError(string)
    # read the file
    with open(string, 'r', encoding='utf-8') as read_file:
        data = json.load(read_file)
    # return the content of the file
    return {'input_file': string, 'json': data}

def uniq(elems: list) -> dict:
    """generate a set of unique objects

    The list is browsed in order to remove duplicates. Each of the unique
    objects has been assigned an identifier to produce a dictionary where keys
    are identifiers and values are objects.

    **Note**: Objects to be compatible with the `uniq()` function has to be
    comparable (`__eq__()`) and has a property called `identifier` which will
    be used in the process of assigning an identifier to the object.

    Args:
        elems: an input `list` of objects

    Returns:
        A dictionary `{identifier: object}`.
    """
    output = {}
    identifier = 0
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
            elem.identifier = identifier
            output[identifier] = elem
            identifier += 1
    return output

def escape(strings: Union[str, list]) -> Union[str, list]:
    """Escape markdown special characters

    Args:
        strings: an input to process

    Return:
        A processed input.

    Raises:
        ValueError: If the input is not `str` nor `list`.
    """
    if isinstance(strings, str):
        return strings.replace('_', '\\_')
    if isinstance(strings, list):
        return [escape(el) for el in strings]
    raise ValueError('Unsupported type in escape(): ' + type(strings))
