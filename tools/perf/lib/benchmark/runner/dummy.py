#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# dummy.py
#

"""a dummy benchmark runner (EXPERIMENTAL)"""

import json
import random

class Dummy:
    """A dummy benchmark runner

    When no actual results are expected.
    """

    @staticmethod
    def __random_point():
        keys = ['threads', 'iodepth', 'bs', 'ops', 'lat_min', 'lat_max',
                'lat_avg', 'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9',
                'lat_pctl_99.99', 'lat_pctl_99.999', 'bw_min', 'bw_max',
                'bw_avg', 'iops_min', 'iops_max', 'iops_avg', 'cpuload']
        return {k: random.randint(0, 10) for k in keys}

    @classmethod
    def run(cls, benchmark, _, idfile: str) -> None:
        """Generate random results

        Args:
            benchmark: the `lib.benchmark.base.Benchmark` object that has
              ordered running a series.
            _: ignored.
            idfile: the output file to store the results.
        """
        if 'rw' in benchmark.oneseries and 'rw' in benchmark.oneseries['rw']:
            output = {
                'read': [cls.__random_point() for _ in range(3)],
                'write': [cls.__random_point() for _ in range(3)]
            }
        else:
            output = [cls.__random_point() for _ in range(3)]
        with open(idfile, 'w', encoding='utf-8') as file:
            json.dump(output, file, indent=4)
