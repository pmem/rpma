#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

"""ib_read_lat.py -- helpers for handling ib_read_lat formats (EXPERIMENTAL)"""

import pandas as pd

class IbReadLatFormat:
    """handling ib_read_lat data"""

    # XXX INPUT_NAMES, OUTPUT_NAMES and read_csv are needed only
    # when using ib_read.sh
    INPUT_NAMES = ['bs', 'ops', 'lat_min', 'lat_max', 'lat_mode', 'lat_avg',
        'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9']

    OUTPUT_NAMES = ['bs', 'ops', 'lat_min', 'lat_max', 'lat_avg', 'lat_stdev',
        'lat_pctl_99.0', 'lat_pctl_99.9']

    @classmethod
    def read_csv(cls, filepath):
        """read a CSV file into pandas.DataFrame"""
        dataframe = pd.read_csv(filepath, header=0, names=cls.INPUT_NAMES)
        return dataframe.reindex(columns=cls.OUTPUT_NAMES)

    @classmethod
    def parse(cls, _output):
        """parse the tool output and return a row of data"""
        # XXX
        return {
            'bs': 0,
            'ops': 0,
            'lat_min': 0,
            'lat_max': 0,
            'lat_avg': 0,
            'lat_stdev': 0,
            'lat_pctl_99.0': 0,
            'lat_pctl_99.9': 0
        }
