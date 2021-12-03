#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# ib_read_bw.py
#

"""ib_read_bw output format tools (EXPERIMENTAL)"""

import pandas as pd
from .ib_read_common import grep_output, line2csv

class IbReadBwFormat:
    """handling ib_read_bw output"""

    # XXX __INPUT_NAMES, __OUTPUT_NAMES and read_csv are needed only
    # when using ib_read.sh
    __INPUT_NAMES = [
        'threads', 'iodepth', 'bs', 'ops', 'bw_peak', 'bw_avg', 'msg_rate']

    __OUTPUT_NAMES = ['threads', 'iodepth', 'bs', 'ops', 'bw_avg']

    @classmethod
    def read_csv(cls, filepath: str) -> pd.DataFrame:
        # pylint: disable=no-member
        # XXX maybe it is no longer a problem for pylint > Debian 9
        """read a CSV file into `pandas.DataFrame`

        Includes:

        - rounding `bw_avg` to 2 decimal places
        - reindex to standardized selection and order of columns

        Args:
            filepath: a path to the CSV file

        Returns:
            The adjusted `pandas.DataFrame`.
        """
        dataframe = pd.read_csv(filepath, header=0, names=cls.__INPUT_NAMES)
        dataframe = dataframe.reindex(columns=cls.__OUTPUT_NAMES)
        return dataframe.apply(lambda x: round(x, 2) \
            if x.name == 'bw_avg' else x)

    @classmethod
    def parse(cls, output : str, pattern : str) -> dict:
        """parse the ib_read_bw output and return a row of data

        Includes:

        - rounding `bw_avg` to 2 decimal places

        Args:
            output: a string collected from the ib_read_bw standard output
            pattern: a pattern to grep for the line with the results

        Returns:
            A `dict` of results.
        """

        line = grep_output(output, pattern)
        csv = line2csv(line)
        data = csv.split(',')

        return {
            'threads': data[0],
            'iodepth': data[1],
            'bs': data[2],
            'ops': data[3],
            'bw_avg': round(data[5], 2)
        }
