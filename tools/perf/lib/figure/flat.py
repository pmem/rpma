#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""flat.py -- helper flattening sets of objects (EXPERIMENTAL)"""

import copy

from .base import Figure

def __str2key(_):
    """Make string a valid jinja2 template key e.g. dictionary.key
    Ref: https://jinja.palletsprojects.com/en/3.0.x/templates/#variables
    """
    return _.replace('.', '')

def __derivatives(oneseries):
    """Generate all derived variables of a series"""
    output = {}
    if 'rw' in oneseries.keys():
        output['rw_order'] = \
            'rand' if oneseries['rw'] in ['randread', 'randwrite'] \
            else 'seq'
    if 'x' in oneseries.keys():
        output['x_key'] = __str2key(oneseries['x'])
    if 'y' in oneseries.keys():
        output['y_key'] = __str2key(oneseries['y'])
    return output

def __make_flat(elems, get, common):
    """Generate all possible flat variants of the elements"""
    # prepend the common part to all elements
    elems = [{**common, **elem} for elem in elems]
    # loop over all elements as long as the set is not flat
    while True:
        any_flatten = False
        output = []
        for elem in elems:
            flattened = False
            for k, values in get(elem).items():
                # fstrings are processed separately
                if k == 'fstrings':
                    continue
                if isinstance(values, list):
                    # each key on the list constitutes a separate output elem
                    for _ in values:
                        ecopy = copy.deepcopy(elem)
                        get(ecopy)[k] = _
                        output.append(ecopy)
                    flattened = True
                    break
            # if the elem is flat it is just passed to the output
            if not flattened:
                output.append(elem)
            else:
                any_flatten = True
        # if no changes happen the output is ready
        if not any_flatten:
            break
        elems = output
    return output

def __process_fstrings(elems, get):
    """Generate final values of parameterized strings"""
    for elem in elems:
        out = get(elem)
        # the default silents an error if the '_comment' key is not present
        out.pop('_comment', None)
        if not 'fstrings' in out.keys():
            continue
        for fstring in out['fstrings']:
            # it is not exactly a fstring-style but it is more convenient
            out[fstring] = out[fstring].format(**out, **__derivatives(out))
        out.pop('fstrings')
    return elems

def __get_figure_desc(figure):
    """Getter for accessing the core descriptor of a figure"""
    return figure['output']

def __get_series_desc(oneseries):
    """Getter for accessing the core descriptor of a series"""
    return oneseries

def flatten(figures):
    """Flatten the figures list"""
    figures = __make_flat(figures, __get_figure_desc, {})
    figures = __process_fstrings(figures, __get_figure_desc)
    output = []
    for figure in figures:
        # flatten series
        common = figure.get('series_common', {})
        figure['series'] = __make_flat(
            figure['series'], __get_series_desc, common)
        figure['series'] = __process_fstrings(
            figure['series'], __get_series_desc)
        output.append(Figure(figure))
    return output
