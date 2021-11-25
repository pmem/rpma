#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""ib_read.py -- the ib_read_{lat,bw} tools runner (EXPERIMENTAL)"""

from shutil import which
from ...common import json_from_file
from ...remote_cmd import RemoteCmd
from .common import UNKNOWN_MODE_MSG, NO_X_AXIS_MSG, BS_VALUES

class IbReadRunner:
    """the ib_read_{lat,bw} tools runner"""

    ONESERIES_REQUIRED = {
        'rw': 'read',
        'filetype': 'malloc'
    }

    def __validate(self):
        """validate the object and readiness of the env"""
        for key, value in self.ONESERIES_REQUIRED.items():
            if key not in self.__benchmark.oneseries:
                raise ValueError(
                    "the following key is missing in the figure: {}"
                    .format(key))
            if self.__benchmark.oneseries[key] != value:
                present_value = self.__benchmark.oneseries[key]
                raise ValueError(".{} == {} != {}".format(key, present_value,
                                                          value))
        if which(self.__settings['ib_tool']) is None:
            raise ValueError("cannot find the local ib tool: {}"
                             .format(self.__settings['ib_tool']))

        # XXX check if self.__settings['ib_tool'] is:
        # - present remotely (using RemoteCmd)

    def __init__(self, benchmark, config, idfile):
        """create a ib_read_* runner object"""
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        self.__server = None
        self.bs_opt = []
        self.qp_opt = []
        self.dp_opt = []
        # pick the settings predefined for the chosen mode
        mode = self.__benchmark.oneseries['mode']
        self.__settings = self.SETTINGS_BY_MODE.get(mode, None)
        if not isinstance(self.__settings, dict):
            raise NotImplementedError(UNKNOWN_MODE_MSG.format(mode))
        # find the x-axis key
        self.__x_key = None
        for x_key in self.X_KEYS:
            if isinstance(self.__settings.get(x_key), list):
                self.__x_key = x_key
                break
        if self.__x_key is None:
            raise NotImplementedError(NO_X_AXIS_MSG.format(mode))
        # load the already collected results
        try:
            self.__results = json_from_file(idfile)
        except FileNotFoundError:
            self.__results = {}
        self.__validate()

    def __set_common_vars(self, _settings):
        """set variables common for server and client"""
        self.bs_opt = ['--size', str(_settings['bs'])]\
                      if _settings['bs_opt'] else []
        self.qp_opt = ['--qp', str(_settings['threads'])]\
                      if _settings['qp_opt'] else []
        self.dp_opt = [''.join(['--tx-depth=', str(_settings['iodepth'])])]\
                      if _settings['dp_opt'] else []

    def __server_start(self, _settings):
        """start a server on the remote side (using RemoteCmd)"""
        # and keep an object allowing to control the server"""
        print('[size: {}, threads: {}, tx_depth: {}, iters: {}] '\
              '(duration: ~60s)'
              .format(_settings['bs'], _settings['threads'],
                      _settings['iodepth'], _settings['iterations']))
        r_numa_n = str(self.__config['REMOTE_JOB_NUMA'])
        r_ib_path = ''.join([self.__config['REMOTE_IB_PATH'],
                            _settings['ib_tool']])
        r_aux_params = [*self.__config['REMOTE_AUX_PARAMS'], *_settings['args']]

        args = ['numactl', '-N', r_numa_n, r_ib_path,
                *self.bs_opt, *self.qp_opt, *self.dp_opt, *r_aux_params]
        cmd = ' '.join(args)
        print("[server]$ {}".format(cmd)) # XXX DEBUG
        # XXX add option to dump the command
        self.__server = RemoteCmd.run_async(self.__config, cmd, None)

    def __server_stop(self, _settings):
        """wait until server finishes"""
        self.__server.wait()
        # XXX write outputs to the LOG_ERR file

    def __client_run(self, _settings):
        # XXX run the client (locally) and wait till the end of execution
        # XXX optionally measure the run time and assert exe_time >= 60s
        # XXX convert the ./csv2standardized.py script into a module?
        # XXX return the measured value
        return 0

    def __result_append(self, x_value, y_value):
        # XXX this part is probably common betwen ib_read and fio classes
        # maybe it should fit into the common module? TBD
        # I think it should also be step-by-step cached into a file
        # so the execution process will be more resilient
        pass

    def __result_is_done(self, x_value):
        """check if the result for a given x value is already collected"""
        # XXX

    def run(self):
        """run the benchmarking sequence"""
        # benchmarks are run for all x values one-by-one
        for x_value in self.__settings[self.__x_key]:
            if self.__result_is_done(x_value):
                continue
            # prepare settings for the current x-axis value
            settings = self.__settings.copy()
            settings[self.__x_key] = x_value
            settings['iterations'] = settings['iterations'].get(x_value, None)
            if settings['iterations'] is None:
                raise NotImplementedError(
                    "settings['iterations'][{}] is missing".format(x_value))
            self.__set_common_vars(settings)
            # XXX remote_command --pre
            self.__server_start(settings)
            y_value = self.__client_run(settings)
            self.__server_stop(settings)
            # XXX remote_command --post
            self.__result_append(x_value, y_value)

    X_KEYS = ['threads', 'bs', 'iodepth']

    SETTINGS_BY_MODE = {
        'lat': {
            'ib_tool': 'ib_read_lat',
            'threads': 1,
            'bs': BS_VALUES,
            'iodepth': 1,
            # values measured empirically, so that duration was ~60s
            'iterations': {
                256:    27678723,
                1024:   27678723,
                4096:   20255739,
                8192:   16778088,
                16384:  11423082,
                32768:  8138946,
                65536:  6002473,
                131072: 3600000,
                262144: 2100000
            },
            'it_opt': True,
            'bs_opt': True,
            'qp_opt': False,
            'dp_opt': False,
            'args': ['--perform_warm_up']}
    }
    # XXX TBD
