#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Benchmark.py -- a single benchmark object (EXPERIMENTAL)
#

import subprocess

class Benchmark:
    """A single benchmark object"""

    def __init__(self, oneseries):
        oneseries['done'] = oneseries.get('done', False)
        self.oneseries = oneseries

    def cache(self):
        return self.oneseries

    def is_done(self):
        return self.oneseries['done']

    def _benchmark_args(self):
        #     - generate a commnd line base on the "b" values
        #     - save results in the hash.json file
        return ['echo', 'XXX']

    def run(self, env):
        args = self._benchmark_args()
        process = subprocess.run(args, env=env)
        # XXX validate process.returncode and fail with a nice error
        self.oneseries['done'] = True
