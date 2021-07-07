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
import subprocess

from .common import *

class Benchmark:
    """A single benchmark object"""

    def __init__(self, oneseries):
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
        return json.JSONEncoder(indent=4).encode(self.oneseries)

    def __eq__(self, other):
        """A comparison function"""
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.oneseries.keys(), *other.oneseries.keys()]))
        for k in keys:
            # ignore series-specific or instance-specific keys
            if k in ['id', 'label']:
                continue
            sv = self.oneseries.get(k, None)
            ov = other.oneseries.get(k, None)
            if sv != ov:
                return False
        return True

    def set_id(self, id):
        """Set an instance id"""
        self.oneseries['id'] = id

    def get_id(self):
        """Get the instance id"""
        return self.oneseries.get('id', None)

    @classmethod
    def uniq(cls, figures):
        """Generate a set of unique benchmarks"""
        output = [cls(oneseries)
            for f in figures
                for oneseries in f.series
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
        output.pop('report', None)
        return output

    def _benchmark_args(self, env):
        if 'tool' not in self.oneseries:
            print("error: 'tool' is missing in the figure")
        if 'mode' not in self.oneseries:
            print("error: 'mode' is missing in the figure")
        if 'server_ip' not in env:
            print("error: 'server_ip' is missing in the environment")
        if 'tool' not in self.oneseries or \
           'mode' not in self.oneseries or \
           'server_ip' not in env:
            raise ValueError
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
            print("error: 'filetype' is missing in the figure")
        if 'id' not in self.oneseries:
            print("error: 'id' is missing in the figure")
        if 'filetype' not in self.oneseries or 'id' not in self.oneseries:
            raise ValueError

        if self.oneseries['filetype'] == 'malloc':
            env['REMOTE_JOB_MEM_PATH'] = 'malloc'
        elif self.oneseries['filetype'] == 'pmem':
            if 'REMOTE_JOB_MEM_PATH' not in env or env['REMOTE_JOB_MEM_PATH'] == 'malloc':
                print("error: 'REMOTE_JOB_MEM_PATH' is not set with a path")
                raise ValueError

        if 'tool_mode' in self.oneseries.keys() and self.oneseries['tool_mode'] == 'gpspm':
            if 'busy_wait_polling' not in self.oneseries:
                print("error: 'busy_wait_polling' is missing in the figure")
                raise ValueError

        if 'busy_wait_polling' in self.oneseries:
            if self.oneseries['busy_wait_polling']:
                env['BUSY_WAIT_POLLING'] = '1'
            else:
                env['BUSY_WAIT_POLLING'] = '0'

        process = subprocess.run(args, env=env)
        process.check_returncode()
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
