#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# ib_read_lat.py
#

"""ib_read_lat output format tools (EXPERIMENTAL)"""


import pandas as pd

class IbReadLatFormat:
    """handling ib_read_lat output"""

    # XXX __INPUT_NAMES, __OUTPUT_NAMES and read_csv are needed only
    # when using ib_read.sh
    __INPUT_NAMES = [
        'bs', 'ops', 'lat_min', 'lat_max', 'lat_mode', 'lat_avg', 'lat_stdev',
        'lat_pctl_99.0', 'lat_pctl_99.9']

    __OUTPUT_NAMES = [
        'bs', 'ops', 'lat_min', 'lat_max', 'lat_avg', 'lat_stdev',
        'lat_pctl_99.0', 'lat_pctl_99.9']

    @classmethod
    def read_csv(cls, filepath: str) -> pd.DataFrame:
        # pylint: disable=no-member
        # XXX maybe it is no longer a problem for pylint > Debian 9
        """read a CSV file into `pandas.DataFrame`

        Includes:

        - reindex to standardized selection and order of columns

        Args:
            filepath: a path to the CSV file

        Returns:
            The `pandas.DataFrame`.
        """
        dataframe = pd.read_csv(filepath, header=0, names=cls.__INPUT_NAMES)
        return dataframe.reindex(columns=cls.__OUTPUT_NAMES)

    @classmethod
    def parse(cls, _output):
        """parse the ib_read_lat output and return a row of data

        Args:
            _output: a string collected from the ib_read_lat standard output

        Returns:
            A `dict`... XXX
        """
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
