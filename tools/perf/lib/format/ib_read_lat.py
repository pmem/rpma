#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

#
# ib_read_lat.py
#

"""ib_read_lat output format tools (EXPERIMENTAL)"""

import re

class IbReadLatFormat:
    """handling ib_read_lat output"""

    @classmethod
    def parse(cls, output: str, pattern: str, _threads: int, \
              _iodepth: int) -> dict:
        """parse the ib_read_lat output and return a row of data

        Args:
            output: a string collected from the ib_read_lat standard output
            pattern: a pattern to grep for the line with the results

        Returns:
            A `dict` of results.

        An example of output (2 lines each broken into 3 parts):
            #bytes  #iterations  t_min[usec]  t_max[usec]  t_typical[usec] \
            256     1000         6.92         14.17        7.13            \

            t_avg[usec]  t_stdev[usec]  99% percentile[usec] \
            7.15         0.29           7.58                 \

            99.9% percentile[usec]
            14.17
        """

        pattern = '([0-9]+)[ ]+([0-9]+)[ ]+([0-9.]+)[ ]+([0-9.]+)[\t ]+'\
                  '([0-9.]+)[\t ]+([0-9.]+)[\t ]+([0-9.]+)[\t ]+([0-9.]+)'\
                  '[\t ]+([0-9.]+)'
        found = re.search(pattern, output)

        return {
            'bs': int(found.group(1)),
            'ops': int(found.group(2)),
            'lat_min': float(found.group(3)),
            'lat_max': float(found.group(4)),
            'lat_avg': float(found.group(6)),
            'lat_stdev': float(found.group(7)),
            'lat_pctl_99.0': float(found.group(8)),
            'lat_pctl_99.9': float(found.group(9))
        }

    @classmethod
    def null_results(cls, blocksize: int, _threads: int, _iodepth: int) -> dict:
        """generate a null ib_read_lat output and return a row of data

        Args:
            blocksize: a value of block size
            _threads: a number of threads
            _iodepth: a value of iodepth

        Returns:
            A `dict` of results.
        """

        return {
            'bs': int(blocksize),
            'ops': 0,
            'lat_min': 0,
            'lat_max': 0,
            'lat_avg': 0,
            'lat_stdev': 0,
            'lat_pctl_99.0': 0,
            'lat_pctl_99.9': 0
        }
