#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# bash.py
#

"""a bash benchmark runner (EXPERIMENTAL)"""

import os
import subprocess

class Bash:
    """A bash benchmark runner

    This runner is always called directly by
    `lib.benchmark.base.Benchmark.run()`.
    """

    @classmethod
    def __benchmark_args(cls, oneseries, config):
        if 'tool' not in oneseries:
            raise ValueError("'tool' is missing in the figure")
        if 'mode' not in oneseries:
            raise ValueError("'mode' is missing in the figure")
        if 'server_ip' not in config:
            raise ValueError("'server_ip' is missing in the configuration")
        args = ['./' + oneseries['tool'], config['server_ip']]
        if 'tool_mode' in oneseries.keys():
            args.append(oneseries['tool_mode'])
        if 'rw' in oneseries.keys():
            args.append(oneseries['rw'])
        args.append(oneseries['mode'])
        return args

    @staticmethod
    def __get_env_value(value):
        if isinstance(value, bool):
            return '1' if value else '0'
        return str(value)

    @classmethod
    def __get_env(cls, config, include_environ=True):
        """Construct the benchmarking environment"""
        # make sure all values are strings
        env = {k: cls.__get_env_value(v) for k, v in config.items()}
        # include:
        # - the parent process environment (optional)
        # - the user-provided configuration
        environ = os.environ if include_environ else {}
        output = {**environ, **env}
        output.pop('_comment', None)
        return output

    @classmethod
    def run(cls, benchmark, config, idfile) -> None:
        """Run the `benchmark` using the external Bash scripts (`ib_read.sh` and
        `rpma_fio_bench.sh`) accompanied by a number of accessory scripts.

        Args:
            benchmark: the `lib.benchmark.base.Benchmark` object that has
              ordered running a series.
            config: the configuration of the benchmarking system.
            idfile: the output file to store the results.

        Raises:
            ValueError: when a misconfiguration is found.
            CalledProcessError: when the benchmark's process returns non-zero
                code.
        """
        args = cls.__benchmark_args(benchmark.oneseries, config)
        env = cls.__get_env(config)

        env['OUTPUT_FILE'] = idfile

        if benchmark.oneseries['filetype'] == 'malloc':
            env['REMOTE_JOB_MEM_PATH'] = 'malloc'
        elif benchmark.oneseries['filetype'] == 'pmem':
            if 'REMOTE_JOB_MEM_PATH' not in env or env['REMOTE_JOB_MEM_PATH'] \
                    == 'malloc':
                raise ValueError("'REMOTE_JOB_MEM_PATH' is not set with a path")

        if 'tool_mode' in benchmark.oneseries.keys() and \
                benchmark.oneseries['tool_mode'] == 'gpspm':
            if 'busy_wait_polling' not in benchmark.oneseries:
                raise ValueError("'busy_wait_polling' is missing in the figure")

        if 'busy_wait_polling' in benchmark.oneseries:
            if benchmark.oneseries['busy_wait_polling']:
                env['BUSY_WAIT_POLLING'] = '1'
            else:
                env['BUSY_WAIT_POLLING'] = '0'

        if 'cpu_load_range' in benchmark.oneseries:
            env['CPU_LOAD_RANGE'] = benchmark.oneseries['cpu_load_range']

        subprocess.run(args, env=env, check=True)
