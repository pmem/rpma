#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# csv2standard.py -- standardize a CSV (EXPERIMENTAL)
#

"""Convert a variety of CSV files into standardized-CSV or JSON format.

The conversion includes:
- renaming headers
- converting units

The standardized format columns are (in order):
- threads - number of threads [1] (optional)
- iodepth - number of operations submitted at once [1] (optional)
- bs - block size [B]
- ops - number of operations executed [1]
- lat_* group (optional)
  - lat_min - min latency [usec]
  - lat_max - max latency [usec]
  - lat_avg - avg. latency [usec]
  - lat_stdev - stdev latency [usec]
  - lat_pctl_99.0 - 99.0th percentile latency [usec]
  - lat_pctl_99.9 - 99.9th percentile latency [usec]
  - lat_pctl_99.99 - 99.99th percentile latency [usec] (optional)
  - lat_pctl_99.999 - 99.999th percentile latency [usec] (optional)
- bw_* group (optional)
  - bw_min - min bandwidth [Gb/sec] (optional)
  - bw_max - max bandwidth [Gb/sec] (optional)
  - bw_avg - avg. bandwidth [Gb/sec]
- iops_* group (optional)
  - iops_min - min [operations/sec] (optional)
  - iops_max - max [operations/sec] (optional)
  - iops_avg - avg. [operations/sec]
- cpuload (optional)
"""

import json
import os.path
from .fio import FioFormat
from .ib_read_bw import IbReadBwFormat
from .ib_read_lat import IbReadLatFormat

class Csv2Standard:
    """standardize a CSV"""

    FORMATS = {
        'ib_lat': IbReadLatFormat,
        'ib_bw': IbReadBwFormat,
    }

    @classmethod
    def __read_csv(cls, csv_file, csv_type):
        """XXX"""
        format_csv = cls.FORMATS.get(csv_type, FioFormat)
        return format_csv.read_csv(csv_file)

    @classmethod
    def convert(cls, csv_files, csv_type, keys, output_file='output.csv'):
        """Standardize a CSV (EXPERIMENTAL)

            Args:
            csv_files - a CSV log file(s) to process
            csv_type - a type of the CSV file ['ib_lat', 'ib_bw', 'fio']
            keys - keys enumerating the input csv_files in the output_file
            output_file - an output file (default='output.csv')
        """

        dfs = [cls.__read_csv(csv_file, csv_type) for csv_file in csv_files]
        _, ext = os.path.splitext(output_file)
        if ext == '.csv':
            if len(dfs) != 1:
                raise ArithmeticError(
                    '''csv output_file does not allow to store more than one input file''')
            dfs[0].to_csv(output_file, index=False)
        elif ext == '.json':
            if keys is None or len(keys) == 0:
                # without keys only a single input file can be stored
                if len(dfs) != 1:
                    raise ArithmeticError(
                        "to store multiple csv_files you have to provide keys")
                dfs[0].to_json(output_file, orient='records')
            else:
                if len(keys) != len(dfs):
                    raise ArithmeticError(
                        '''the number of keys have to be equal to the number of csv_files''')
                # prepare the output dict()
                output = {key: df.to_dict(orient='records')
                          for key, df in zip(keys, dfs)}
                # write the constructed dict() to the output_file
                with open(output_file, 'w', encoding='utf-8') as file:
                    json.dump(output, file)
        else:
            raise Exception("Unsupported output file extension: {}".format(ext))
