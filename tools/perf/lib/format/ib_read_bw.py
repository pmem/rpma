#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

"""ib_read_bw.py -- helpers for handling ib_read_bw formats (EXPERIMENTAL)"""

import pandas as pd

class IbReadBwFormat:
    """handling ib_read_bw data"""

    # XXX INPUT_NAMES, OUTPUT_NAMES and read_csv are needed only
    # when using ib_read.sh
    INPUT_NAMES = ['threads', 'iodepth', 'bs', 'ops', 'bw_peak', 'bw_avg',
        'msg_rate']

    OUTPUT_NAMES = ['threads', 'iodepth', 'bs', 'ops', 'bw_avg']

    @classmethod
    def read_csv(cls, filepath):
        """read a CSV file into pandas.DataFrame"""
        dataframe = pd.read_csv(filepath, header=0, names=cls.INPUT_NAMES)
        dataframe = dataframe.reindex(columns=cls.OUTPUT_NAMES)
        return dataframe.apply(lambda x: round(x, 2) \
            if x.name == 'bw_avg' else x)

    @classmethod
    def parse(cls, _output):
        """parse the tool output and return a row of data"""
        # XXX
        return {
            'threads': 0,
            'iodepth': 0,
            'bs': 0,
            'ops': 0,
            'bw_avg': 0
        }
