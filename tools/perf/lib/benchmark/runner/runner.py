#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# runner.py
#

"""the abstract runner (EXPERIMENTAL)"""

import json
import re

from .common import MISSING_KEY_MSG
from ...remote_cmd import RemoteCmd


class Runner:
    """the abstract runner

    The abstract runner covers common runners functionality.
    """

    def __init__(self, benchmark, config: dict, idfile: str) -> 'Runner':
        # XXX nice to have REMOTE_JOB_NUMA_CPULIST, CORES_PER_SOCKET
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        self.__data = []
        self.__validate()
        # set dumping commands
        self.__dump_cmds = self._config.get('DUMP_CMDS', False)


    def __validate(self):
        if self._benchmark is None:
            raise RuntimeError("Benchmark is missing")

        if self._oneseries is None:
            raise RuntimeError("OneSeries is missing")

        for key in self.__ONESERIES_REQUIRED:
            if key not in self._benchmark.oneseries:
                raise ValueError(MISSING_KEY_MSG.format(key))

        if self._config is None:
            raise RuntimeError("Config is missing")

        for key in self.__CONFIG_REQUIRED:
            if key not in self._config:
                raise ValueError(MISSING_KEY_MSG.format(key))

    @property
    def _benchmark(self):
        return self.__benchmark

    @property
    def _config(self):
        return self.__config

    @property
    def _idfile(self) -> str:
        return self.__idfile

    @property
    def _data(self):
        return self.__data
    @_data.setter
    def _data(self, data):
        self.__data = data

    @property
    def _oneseries(self):
        return self._benchmark.oneseries

    @property
    def _dump_cmds(self):
        return self.__dump_cmds

    @property
    def _mode(self):
        return self._oneseries['mode']

    @property
    def _tool_mode(self):
        return self._oneseries['tool_mode']

    @property
    def _tool(self):
        return self._oneseries['tool']

    def _print_start_message(self):
        """print the STARTING message"""
        tool = self._tool + '({})'.format(self._tool_mode)
        print('STARTING benchmark TOOL={} for MODE={} (IP={}) ...'
              .format(tool, self._mode, self._config['server_ip']))

    def _result_append(self, result: dict) -> None:
        """append new result to internal data and file"""
        self._data.append(result)
        with open(self._idfile, 'w', encoding='utf-8') as file:
            json.dump(self._data, file, indent=4)

    def _result_is_done(self, x_key: str, x_value: int) -> bool:
        """check if the result for the given x_value of x_key is already collected"""
        for result in self._data:
            # A result can be a tuple (read, write) or a list of two elements
            # in case of the fio 'rw' mode and then it is enough to check
            # only the first item, because they both have to have the same keys.
            if isinstance(result, (tuple, list)):
                result = result[0]
            if x_key not in result:
                raise ValueError('key \'{}\' is missing the previous results'
                                 .format(x_key))
            if str(result[x_key]) == str(x_value):
                return True
        return False

    def __prepare_cmd(self, cmd_exec, x_value=None):
        """prepare cmd"""
        cmd = cmd_exec
        cmd_vars = re.findall(r'\${.+?}', cmd)
        for item in cmd_vars:
            replace_item = item.replace('${', '').replace('}', '')
            if replace_item in self._config:
                cmd = cmd.replace(item, str(self._config[replace_item]))
            elif replace_item == 'RUN_NAME' and x_value is not None:
                run_name = 'benchmark_{}_x_value_{}'\
                            .format(self._oneseries['id'], x_value)
                cmd = cmd.replace(item, run_name)
        return cmd

    def _run_pre_command(self, x_value):
        """run pre command"""
        run_pre = None
        if 'REMOTE_CMD_PRE' in self._config and \
                                        self._config['REMOTE_CMD_PRE'] != '':
            cmd = self.__prepare_cmd(self._config['REMOTE_CMD_PRE'], x_value)
            run_pre = RemoteCmd.run_async(self._config, cmd)
        return run_pre

    def __wait_for_pre_command(self, pre_command, raise_on_error=True):
        """wait for the end of the pre command"""
        if pre_command is None:
            return
        print('Waiting for pre-command ...')
        pre_command.wait()
        if pre_command.exit_status == 0:
            return
        print('--- pre-command\'s stderr: ---')
        stderr_msg = pre_command.stderr.read().decode().strip()
        if raise_on_error:
            raise ValueError(stderr_msg)
        print(stderr_msg)
        print('--- end of pre-command\'s stderr ---\n')

    def _run_post_command(self, pre_command=None):
        """run post command and wait for the end of the pre command"""
        if 'REMOTE_CMD_POST' in self._config and \
                                        self._config['REMOTE_CMD_POST'] != '':
            cmd = self.__prepare_cmd(self._config['REMOTE_CMD_POST'])
            try:
                RemoteCmd.run_sync(self._config, cmd, raise_on_error=True)
            except:
                print('Post-command failed!')
                self.__wait_for_pre_command(pre_command, raise_on_error=False)
                print('--- post-command\'s output: ---')
                raise
        self.__wait_for_pre_command(pre_command)

    __ONESERIES_REQUIRED = ['tool', 'tool_mode', 'mode']
    __CONFIG_REQUIRED = ['server_ip']
