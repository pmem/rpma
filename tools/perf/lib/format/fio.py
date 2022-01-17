#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

#
# fio.py
#

"""FIO JSON+ output format tools (EXPERIMENTAL)"""

import json
import random

class FioFormat:
    """handling FIO output"""

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
    def __job_op_standardized(cls, op_result):
        lat_ns = op_result['lat_ns']
        lat_pctl_ns = lat_ns.get('percentile', {})
        output = {
            'threads': int(op_result['numjobs']),
            'iodepth': int(op_result['iodepth']),
            'bs': int(op_result['bs']),
            'ops': int(lat_ns['N']),
            'lat_min': float(lat_ns['min']),
            'lat_max': float(lat_ns['max']),
            'lat_avg': float(lat_ns['mean']),
            'lat_stdev': float(lat_ns['stddev']),
            'lat_pctl_99.0': float(lat_pctl_ns.get('99.000000', 0)),
            'lat_pctl_99.9': float(lat_pctl_ns.get('99.900000', 0)),
            'lat_pctl_99.99': float(lat_pctl_ns.get('99.990000', 0)),
            'lat_pctl_99.999': float(lat_pctl_ns.get('99.999000', 0)),
            'bw_min': float(op_result['bw_min']),
            'bw_max': float(op_result['bw_max']),
            'bw_avg': float(op_result['bw']),
            'iops_min': float(op_result['iops_min']),
            'iops_max': float(op_result['iops_max']),
            'iops_avg': float(op_result['iops'])
        }
        # convert nsec to usec
        for key in cls.__NSEC_2_USEC_NAMES:
            output[key] = cls.__convert_nsec_to_usec(output[key])
        # convert KiB/s to Gb/s
        for key in cls.__KiBps_2_Gbps_NAMES:
            output[key] = cls.__convert_KiBps_to_Gbps(output[key])
        return output

    @classmethod
    def parse(cls, json_str: str) -> dict:
        """parse the FIO JSON+ output and return a row of data

        The row of data on the output of this function looks as follow:

        ```python
        {
            'read': {
                # the results
            },
            'write': {
                # the results
            }
        }
        ```

        Where `the results` are according the chosen `output_format`.

        Args:
            json_str: a string collected from the FIO standard output

        Returns:
            The row of data.
        """
        job = json.loads(json_str)['jobs'][0]
        options = job['job options']
        read = {**job['read'], **options}
        write = {**job['write'], **options}
        job_op_format = cls.__job_op_standardized
        return {
            'read': job_op_format(read),
            'write': job_op_format(write)
        }

    @classmethod
    def __generate_random_results(cls, env):
        return {
            'threads': int(env['numjobs']),
            'iodepth': int(env['iodepth']),
            'bs': int(env['blocksize']),
            'ops': random.randint(0, 10),
            'lat_min': random.randint(0, 10),
            'lat_max': random.randint(0, 10),
            'lat_avg': random.randint(0, 10),
            'lat_stdev': random.randint(0, 10),
            'lat_pctl_99.0': random.randint(0, 10),
            'lat_pctl_99.9': random.randint(0, 10),
            'lat_pctl_99.99': random.randint(0, 10),
            'lat_pctl_99.999': random.randint(0, 10),
            'bw_min': random.randint(0, 10),
            'bw_max': random.randint(0, 10),
            'bw_avg': random.randint(0, 10),
            'iops_min': random.randint(0, 10),
            'iops_max': random.randint(0, 10),
            'iops_avg': random.randint(0, 10)
        }

    @classmethod
    def random_results(cls, env: dict) -> dict:
        """generate random results of the FIO JSON+ output
           and return a row of data

        The row of data on the output of this function looks as follow:

        ```python
        {
            'read': {
                # random results
            },
            'write': {
                # random results
            }
        }
        ```

        Args:
            env: an environment dictionary

        Returns:
            The row of data.
        """
        random_results = cls.__generate_random_results(env)
        return {
            'read': random_results,
            'write': random_results
        }
