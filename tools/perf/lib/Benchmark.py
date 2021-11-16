#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Benchmark.py -- a single benchmark object (EXPERIMENTAL)
#

import json
import os
import random
import subprocess

from .common import *

ENCODE = json.JSONEncoder(indent=4).encode

class Benchmark:
    """A single benchmark object"""

    def __init__(self, oneseries, from_figure=False):
        self.oneseries_from_figure = None
        if from_figure:
            # When oneseries belongs to a Figure:
            # - it should not be modified directly (a copy is required) and
            # - the id when set should be propagated to the original oneseries 
            #   object to allow relating among a unique Benchmark
            #   and potentially multiple series making use of it.
            self.oneseries_from_figure = oneseries
            oneseries = oneseries.copy()
            # validate if it is a mixed workload
            if 'rw' in oneseries and 'rw' in oneseries['rw']:
                if 'rw_dir' not in oneseries:
                    raise SyntaxError(
                        "'rw_dir' is required for mixed workloads (['rw'] == '*rw')\n{}".format(
                            ENCODE(oneseries)))
            # remove unnecessary fields
            oneseries.pop('label', None)
            oneseries.pop('rw_dir', None)
        oneseries['done'] = oneseries.get('done', False)
        self.oneseries = oneseries
        if 'requirements' in oneseries.keys():
            self.req = oneseries['requirements']
            # remove the remaining duplicate
            oneseries.pop('requirements')
        else:
            self.req = {}

    def __repr__(self):
        """A string representation of the object"""
        return ENCODE(self.oneseries)

    def __eq__(self, other):
        """A comparison function"""
        # The same benchmark run under different requirements
        # are two different benchmarks.
        if self.req != other.req:
            return False
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.oneseries.keys(), *other.oneseries.keys()]))
        for k in keys:
            # ignore series-specific or instance-specific keys
            if k == 'id':
                continue
            sv = self.oneseries.get(k, None)
            ov = other.oneseries.get(k, None)
            if sv != ov:
                return False
        return True

    def set_id(self, id):
        """Set an instance id"""
        self.oneseries['id'] = id
        if self.oneseries_from_figure is not None:
            # Propagated to the original oneseries object to allow relating
            # to the Benchmark object.
            self.oneseries_from_figure['id'] = id

    def get_id(self):
        """Get the instance id"""
        return self.oneseries.get('id', None)

    @classmethod
    def uniq(cls, figures):
        """Generate a set of unique benchmarks"""
        output = [cls(oneseries, from_figure=True)
            for f in figures
                for oneseries in f.get_series_in()
        ]
        return uniq(output)

    def get_requirements(self):
        return self.req

    def cache(self):
        """Cache the current state of execution"""
        return self.oneseries

    def is_done(self):
        return self.oneseries['done']

    def _get_env_value(value):
        if type(value) is bool:
            return '1' if value else '0'
        else:
            return str(value)

    def _get_env(self, config, result_dir, include_environ=True):
        """Construct the benchmarking environment"""
        # make sure all values are strings
        env = {k: Benchmark._get_env_value(v) for k, v in config.items()}
        output_file = os.path.join(result_dir,
            'benchmark_' + str(self.get_id()) + '.json')
        # include:
        # - the parent process environment (optional)
        # - the user-provided configuration
        # - the output file path
        environ = os.environ if include_environ else {}
        output = {**environ, **env, **{'OUTPUT_FILE': output_file}}
        output.pop('_comment', None)
        return output

    def _benchmark_args(self, env):
        if 'tool' not in self.oneseries:
            raise ValueError("'tool' is missing in the figure")
        if 'mode' not in self.oneseries:
            raise ValueError("'mode' is missing in the figure")
        if 'server_ip' not in env:
            raise ValueError("'server_ip' is missing in the configuration")
        args = ['./' + self.oneseries['tool'], env['server_ip']]
        if 'tool_mode' in self.oneseries.keys():
            args.append(self.oneseries['tool_mode'])
        if 'rw' in self.oneseries.keys():
            args.append(self.oneseries['rw'])
        args.append(self.oneseries['mode'])
        return args

    def run(self, config, result_dir):
        """Run the benchmark process and mark it as done.

        Args:
            config (dict): a user-provided system config
            result_dir (str): the directory for the benchmark's results
        Raises:
            CalledProcessError: when the benchmark's process returns non-zero
                code
        Returns:
            None
        """
        args = self._benchmark_args(config)
        env = self._get_env(config, result_dir)

        if 'filetype' not in self.oneseries:
            raise ValueError("'filetype' is missing in the figure")
        if 'id' not in self.oneseries:
            raise ValueError("'id' is missing in the figure")

        if self.oneseries['filetype'] == 'malloc':
            env['REMOTE_JOB_MEM_PATH'] = 'malloc'
        elif self.oneseries['filetype'] == 'pmem':
            if 'REMOTE_JOB_MEM_PATH' not in env or env['REMOTE_JOB_MEM_PATH'] == 'malloc':
                raise ValueError("'REMOTE_JOB_MEM_PATH' is not set with a path")

        if 'tool_mode' in self.oneseries.keys() and self.oneseries['tool_mode'] == 'gpspm':
            if 'busy_wait_polling' not in self.oneseries:
                raise ValueError("'busy_wait_polling' is missing in the figure")

        if 'busy_wait_polling' in self.oneseries:
            if self.oneseries['busy_wait_polling']:
                env['BUSY_WAIT_POLLING'] = '1'
            else:
                env['BUSY_WAIT_POLLING'] = '0'

        if 'cpu_load_range' in self.oneseries:
            env['CPU_LOAD_RANGE'] = self.oneseries['cpu_load_range']

        if config.get('dummy_results', False):
            self._run_dummy(env)
        else:
            process = subprocess.run(args, env=env)
            process.check_returncode()
        self.oneseries['done'] = True

    def skip(self):
        self.oneseries['done'] = True

    def dump(self, config, result_dir):
        args = self._benchmark_args(config)
        env = self._get_env(config, result_dir, include_environ=False)
        id = self.get_id()
        done = 'done' if self.is_done() else 'not done'
        print("Benchmark[{}]: {}".format(id, done))
        print('- Environment:')
        print("\n".join(["{}=\"{}\"".format(k, v) for k, v in env.items()]))
        print('- Command: ' + ' '.join(args))

    def _random_point(self):
        keys = ['threads', 'iodepth', 'bs', 'ops', 'lat_min', 'lat_max',
            'lat_avg', 'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9',
            'lat_pctl_99.99', 'lat_pctl_99.999', 'bw_min', 'bw_max', 'bw_avg',
            'iops_min', 'iops_max', 'iops_avg', 'cpuload']
        return {k: random.randint(0, 10) for k in keys}

    def _run_dummy(self, env):
        if 'rw' in self.oneseries and 'rw' in self.oneseries['rw']:
            output = {
                'read': [self._random_point() for i in range(3)],
                'write': [self._random_point() for i in range(3)]
            }
        else:
            output = [self._random_point() for i in range(3)]
        with open(env['OUTPUT_FILE'], 'w', encoding="utf-8") as file:
            json.dump(output, file, indent=4)
