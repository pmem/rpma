#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""base.py -- the base benchmark runner (EXPERIMENTAL)"""

from ...common import get_benchmark_result_path
from .fio import FioRunner
from .ib_read import IbReadRunner

class BaseRunner:
    """XXX"""

    RUNNERS = {
        'fio': FioRunner,
        'ib_read': IbReadRunner
    }

    @classmethod
    def run(cls, benchmark, config, result_dir):
        """XXX"""
        # XXX DUMP_CMDS?
        idfile = get_benchmark_result_path(result_dir, benchmark.identifier)
        runner_cls = cls.RUNNERS.get(benchmark.oneseries['tool'], None)
        if runner_cls is None:
            raise NotImplementedError()
        runner = runner_cls(benchmark, config, idfile)
        runner.run()
