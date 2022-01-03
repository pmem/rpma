#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# executor.py
#

"""the benchmark runner executor (EXPERIMENTAL)"""

from .fio import FioRunner
from .ib_read import IbReadRunner

class Executor:
    """The benchmark runner executor

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
        if 'tool' not in benchmark.oneseries:
            raise ValueError('\'tool\' key is missing in the figure')
        tool = benchmark.oneseries['tool']
        runner_cls = cls.__RUNNERS.get(tool, None)
        if runner_cls is None:
            raise NotImplementedError('\'{}\' runner is not implemented'
                                      .format(tool))
        runner = runner_cls(benchmark, config, idfile)
        runner.run()
