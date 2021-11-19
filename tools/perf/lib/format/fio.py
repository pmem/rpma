#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

"""fio.py -- helpers for handling FIO formats (EXPERIMENTAL)"""

import pandas as pd

class FioFormat:
    """handling FIO data"""

    # XXX INPUT_NAMES, OUTPUT_NAMES and read_csv are needed only when using
    # rpma_fio_bench.sh
    INPUT_NAMES = ['bs', 'iodepth', 'threads', 'lat_min', 'lat_max', 'lat_avg',
        'lat_stdev', 'ops', 'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99',
        'lat_pctl_99.999', 'bw_avg', 'bw_min', 'bw_max', 'iops_avg', 'iops_min',
        'iops_max', 'cpuload']

    OUTPUT_NAMES = ['threads', 'iodepth', 'bs', 'ops', 'lat_min', 'lat_max',
        'lat_avg', 'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9',
        'lat_pctl_99.99', 'lat_pctl_99.999', 'bw_min', 'bw_max', 'bw_avg',
        'iops_min', 'iops_max', 'iops_avg', 'cpuload']

    NSEC_2_USEC_NAMES = ['lat_min', 'lat_max', 'lat_avg', 'lat_stdev',
        'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

    KiBps_2_Gbps_NAMES = ['bw_avg', 'bw_min', 'bw_max']

    KiBps_2_Gbps = 1024 * 8 / 1000 / 1000 / 1000

    @classmethod
    def read_csv(cls, filepath):
        """read a CSV file into pandas.DataFrame"""
        dataframe = pd.read_csv(filepath, header=0, names=cls.INPUT_NAMES)
        # convert nsec to usec
        dataframe = dataframe.apply(lambda x: round(x / 1000, 2) \
            if x.name in cls.NSEC_2_USEC_NAMES else x)
        # convert KiB/s to Gb/s
        dataframe = dataframe.apply(lambda x: round(x * cls.KiBps_2_Gbps, 2) \
            if x.name in cls.KiBps_2_Gbps_NAMES else x)
        return dataframe.reindex(columns=cls.OUTPUT_NAMES)

    @classmethod
    def parse(cls, filepath):
        """parse the FIO JSON+ output and return a row of data"""
        # XXX a good portion of this code will be probably based on
        # the fio_json2csv.py script.
        return {
            'threads': 0,
            'iodepth': 0,
            'bs': 0,
            'ops': 0,
            'bw_avg': 0
        }
