#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# flat.py
#

"""figure list flattening tools (EXPERIMENTAL)

A 'flat' list of figures is a list where:

- a single element in this list represents exactly one output figure and
- all elements in `figure['series']` lists represent exactly one series of
  benchmarks.

By default, figures in part's JSON files may be multidimensional.
A multidimensional figure is a figure in which metadata or
one of its series is described by a list instead of a 'flat' set of values.
"""

import copy

from .base import Figure
from ..common import ENCODE

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

__MISSING_RW_DIR_MSG = """'rw_dir' is required for mixed workloads (['rw'] == '*rw')\n{}"""

def __validate_series(series):
    for oneseries in series:
        # validate mixed workload
        if 'rw' in oneseries and 'rw' in oneseries['rw']:
            if 'rw_dir' not in oneseries:
                raise SyntaxError(
                    __MISSING_RW_DIR_MSG.format(ENCODE(oneseries)))

def flatten(figures: list, result_dir: str) -> list:
    """Flatten a figures list

    Args:
        figures: A list of figures (dictionaries not `lib.figure.base.Figure`
          instances) where each of the figures and each of the series can be
          multidimensional.

        result_dir: a directory where the intermediate and final products of
          the benchmarking process will be stored.
    Returns:
        A list of `lib.figure.base.Figure` objects being a flattened
        representation of the provided `figures` list.
    """
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
        __validate_series(figure['series'])
        output.append(Figure(figure, result_dir))
    return output
