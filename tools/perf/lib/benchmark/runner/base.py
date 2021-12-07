#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py
#

"""the base benchmark runner (EXPERIMENTAL)"""

from .fio import FioRunner
from .ib_read import IbReadRunner

class BaseRunner:
    """The base benchmark runner

    Depending on the workload to run it either invokes
    `lib.benchmark.runner.fio` or `lib.benchmark.runner.ib_read`.
    """

    __RUNNERS = {
        'fio': FioRunner,
        'ib_read': IbReadRunner
    }

    @classmethod
    def run(cls, benchmark, config: dict, idfile: str) -> None:
        """Run the given `benchmark`

        Args:
            benchmark: the `lib.benchmark.base.Benchmark` object that has
              ordered running a series.
            config: the configuration of the benchmarking system.
            idfile: the output file to store the results.
        """
        runner_cls = cls.__RUNNERS.get(benchmark.oneseries['tool'], None)
        if runner_cls is None:
            raise NotImplementedError()
        runner = runner_cls(benchmark, config, idfile)
        runner.run()
