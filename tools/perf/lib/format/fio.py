#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# fio.py
#

"""FIO JSON+ output format tools (EXPERIMENTAL)"""

from enum import Enum

import json
import pandas as pd

class Output(Enum):
    """supported output formats"""
    #: should NOT be used outside `fio_json2csv`
    DUMP = 1

    #: a standard results format
    STANDARDIZED = 2

class FioFormat:
    """handling FIO output"""

    # XXX __INPUT_NAMES, __OUTPUT_NAMES and read_csv are needed only when using
    # rpma_fio_bench.sh
    __INPUT_NAMES = [
        'bs', 'iodepth', 'threads', 'lat_min', 'lat_max', 'lat_avg',
        'lat_stdev', 'ops', 'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99',
        'lat_pctl_99.999', 'bw_avg', 'bw_min', 'bw_max', 'iops_avg', 'iops_min',
        'iops_max', 'cpuload']

    __OUTPUT_NAMES = [
        'threads', 'iodepth', 'bs', 'ops', 'lat_min', 'lat_max', 'lat_avg',
        'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99',
        'lat_pctl_99.999', 'bw_min', 'bw_max', 'bw_avg', 'iops_min', 'iops_max',
        'iops_avg', 'cpuload']

    __NSEC_2_USEC_NAMES = [
        'lat_min', 'lat_max', 'lat_avg', 'lat_stdev', 'lat_pctl_99.0',
        'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

    __KiBps_2_Gbps_NAMES = ['bw_avg', 'bw_min', 'bw_max']

    __KiBps_2_Gbps = 1024 * 8 / 1000 / 1000 / 1000

    @staticmethod
    def __convert_nsec_to_usec(value):
        """convert [nsec] value to [usec]"""
        return round(value / 1000, 2)

    @classmethod
    def __convert_KiBps_to_Gbps(cls, value):
        # KiBps and Gbps are case-sensitive
        # pylint: disable=invalid-name
        """convert [KiB/s] value to [Gb/s]"""
        return round(value * cls.__KiBps_2_Gbps, 2)

    @classmethod
    def read_csv(cls, filepath: str) -> pd.DataFrame:
        # pylint: disable=no-member
        # XXX maybe it is no longer a problem for pylint > Debian 9
        """read a CSV file into `pandas.DataFrame`

        Includes:

        - converting [nsec] to [usec]
        - converting [KiBps] to [Gbps]
        - reindex to standardized selection and order of columns

        Args:
            filepath: a path to the CSV file

        Returns:
            The adjusted `pandas.DataFrame`.
        """
        dataframe = pd.read_csv(
            filepath, header=0, names=cls.__INPUT_NAMES)
        # convert nsec to usec
        dataframe = dataframe.apply(lambda x: cls.__convert_nsec_to_usec(x) \
            if x.name in cls.__NSEC_2_USEC_NAMES else x)
        # convert KiB/s to Gb/s
        dataframe = dataframe.apply(lambda x: cls.__convert_KiBps_to_Gbps(x) \
            if x.name in cls.__KiBps_2_Gbps_NAMES else x)
        return dataframe.reindex(columns=cls.__OUTPUT_NAMES)

    @classmethod
    def __job_op_dump(cls, op_result):
        lat_ns = op_result['lat_ns']
        lat_pctl_ns = lat_ns.get('percentile', {})
        return {
            'bs': op_result['bs'],
            'iodepth': op_result['iodepth'],
            'threads': op_result['numjobs'],
            'lat_ns_min': lat_ns['min'],
            'lat_ns_max': lat_ns['max'],
            'lat_ns_mean': lat_ns['mean'],
            'lat_ns_stddev': lat_ns['stddev'],
            'ops': lat_ns['N'],
            'lat_ns_pctl_99.0': lat_pctl_ns.get('99.000000', 0),
            'lat_ns_pctl_99.9': lat_pctl_ns.get('99.900000', 0),
            'lat_ns_pctl_99.99': lat_pctl_ns.get('99.990000', 0),
            'lat_ns_pctl_99.999': lat_pctl_ns.get('99.999000', 0),
            'bw_avg': op_result['bw'],
            'bw_min': op_result['bw_min'],
            'bw_max': op_result['bw_max'],
            'iops_avg': op_result['iops'],
            'iops_min': op_result['iops_min'],
            'iops_max': op_result['iops_max']
        }

    @classmethod
    def __job_op_standardized(cls, op_result):
        lat_ns = op_result['lat_ns']
        lat_pctl_ns = lat_ns.get('percentile', {})
        output = {
            'threads': op_result['numjobs'],
            'iodepth': op_result['iodepth'],
            'bs': op_result['bs'],
            'ops': lat_ns['N'],
            'lat_min': lat_ns['min'],
            'lat_max': lat_ns['max'],
            'lat_avg': lat_ns['mean'],
            'lat_stdev': lat_ns['stddev'],
            'lat_pctl_99.0': lat_pctl_ns.get('99.000000', 0),
            'lat_pctl_99.9': lat_pctl_ns.get('99.900000', 0),
            'lat_pctl_99.99': lat_pctl_ns.get('99.990000', 0),
            'lat_pctl_99.999': lat_pctl_ns.get('99.999000', 0),
            'bw_min': op_result['bw_min'],
            'bw_max': op_result['bw_max'],
            'bw_avg': op_result['bw'],
            'iops_min': op_result['iops_min'],
            'iops_max': op_result['iops_max'],
            'iops_avg': op_result['iops']
        }
        # convert nsec to usec
        for key in cls.__NSEC_2_USEC_NAMES:
            output[key] = cls.__convert_nsec_to_usec(output[key])
        # convert KiB/s to Gb/s
        for key in cls.__KiBps_2_Gbps_NAMES:
            output[key] = cls.__convert_KiBps_to_Gbps(output[key])
        return output

    @classmethod
    def parse(cls, json_str: str, output_format=Output.STANDARDIZED) -> dict:
        """parse the FIO JSON+ output and return a row of data

        Args:
            json_str: a string collected from the FIO standard output
            output_format: a `Output` value.

        Returns:
            A `dict` containing two keys: `'read'` and `'write'`. Under each of
            the storead a 2nd level `dict` with all the results according to
            chosen `output_format`.
        """
        job = json.loads(json_str)['jobs'][0]
        opions = job['job options']
        read = {**job['read'], **opions}
        write = {**job['write'], **opions}
        formats = {
            Output.DUMP: cls.__job_op_dump,
            Output.STANDARDIZED: cls.__job_op_standardized
        }
        job_op_format = formats[output_format]
        return {
            'read': job_op_format(read),
            'write': job_op_format(write)
        }
