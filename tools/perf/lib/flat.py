#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# flat.py -- helper flattening sets of objects (EXPERIMENTAL)
#

import copy

def make_flat(elems, get, common={}):
    """Generate all possible flat variants of the elements"""
    # prepend the common part to all elements
    elems = [{**common, **elem} for elem in elems]
    # loop over all elements as long as the set is not flat
    while True:
        any_flatten = False
        output = []
        for elem in elems:
            flatten = False
            for k, vs in get(elem).items():
                # fstrings are processed separately
                if k == 'fstrings':
                    continue
                if type(vs) is list:
                    # each key on the list constitutes a separate output elem
                    for v in vs:
                        ecopy = copy.deepcopy(elem)
                        get(ecopy)[k] = v
                        output.append(ecopy)
                    flatten = True
                    break
            # if the elem is flat it is just passed to the output
            if not flatten:
                output.append(elem)
            else:
                any_flatten = True
        # if no changes happen the output is ready
        if not any_flatten:
            break
        else:
            elems = output
    return output

def empty_dict(input):
    return {}

def process_fstrings(elems, get, derivatives=empty_dict):
    """Generate final values of parameterized strings"""
    for elem in elems:
        out = get(elem)
        if not 'fstrings' in out.keys():
            continue
        for fstring in out['fstrings']:
            # it is not exactly a fstring-style but it is more convenient
            out[fstring] = out[fstring].format(**out, **derivatives(out))
        out.pop('fstrings')
        # the default silents an error if the '_comment' key is not present
        out.pop('_comment', None)
    return elems
