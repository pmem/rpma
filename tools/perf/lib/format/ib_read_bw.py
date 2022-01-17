#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

#
# ib_read_bw.py
#

"""ib_read_bw output format tools (EXPERIMENTAL)"""

import random
import re

class IbReadBwFormat:
    """handling ib_read_bw output"""

    @classmethod
    def parse(cls, output: str, pattern: str, threads: int, iodepth: int) \
              -> dict:
        """parse the ib_read_bw output and return a row of data

        Includes:

        - rounding `bw_avg` to 2 decimal places

        Args:
            output: a string collected from the ib_read_bw standard output
            pattern: a pattern to grep for the line with the results

        Returns:
            A `dict` of results.

        An example of output (2 lines each broken into 2 parts):
            #bytes  #iterations  BW peak[Gb/sec]  BW average[Gb/sec] \
            256     1000         0.28             0.27               \

            MsgRate[Mpps]
            0.130056
        """

        pattern = \
            '([0-9]+)[ ]+([0-9]+)[ ]+([0-9.]+)[ ]+([0-9.]+)[\t ]+([0-9.]+)'
        found = re.search(pattern, output)

        return {
            'threads': int(threads),
            'iodepth': int(iodepth),
            'bs': int(found.group(1)),
            'ops': int(found.group(2)),
            'bw_avg': round(float(found.group(4)), 2)
        }

    @classmethod
    def random_results(cls, blocksize: int, threads: int, iodepth: int) -> dict:
        """generate a random ib_read_bw output and return a row of data

        Args:
            blocksize: a value of block size
            threads: a number of threads
            iodepth: a value of iodepth

        Returns:
            A `dict` of results.
        """

        return {
            'threads': int(threads),
            'iodepth': int(iodepth),
            'bs': int(blocksize),
            'ops': random.randint(0, 10),
            'bw_avg': random.randint(0, 10)
        }
