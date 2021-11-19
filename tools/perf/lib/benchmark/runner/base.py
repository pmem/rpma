#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""base.py -- the base benchmark runner (EXPERIMENTAL)"""

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
        idfile = benchmark.get_output_file(result_dir)
        runner_cls = cls.RUNNERS.get(benchmark.oneseries['tool'], None)
        if runner_cls is None:
            raise NotImplementedError()
        runner = runner_cls(benchmark, config, idfile)
        runner.run()
