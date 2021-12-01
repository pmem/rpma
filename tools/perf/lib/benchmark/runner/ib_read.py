#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""ib_read.py -- the ib_read_{lat,bw} tools runner (EXPERIMENTAL)"""

from datetime import datetime
from os.path import join
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

        ib_path = join(self.__config.get('REMOTE_IB_PATH', ''),
                       self.__settings['ib_tool'])
        output = RemoteCmd.run_sync(self.__config, ['which', ib_path], None)
        if output.exit_status != 0:
            raise ValueError("cannot find the remote ib tool: {}"
                             .format(self.__settings['ib_tool']))

    def __init__(self, benchmark, config, idfile):
        """create a ib_read_* runner object"""
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        self.__server = None
        # pick the settings predefined for the chosen mode
        self.__mode = self.__benchmark.oneseries['mode']
        self.__settings = self.SETTINGS_BY_MODE.get(self.__mode, None)
        if not isinstance(self.__settings, dict):
            raise NotImplementedError(UNKNOWN_MODE_MSG.format(self.__mode))
        # find the x-axis key
        self.__x_key = None
        for x_key in self.X_KEYS:
            if isinstance(self.__settings.get(x_key), list):
                self.__x_key = x_key
                break
        if self.__x_key is None:
            raise NotImplementedError(NO_X_AXIS_MSG.format(self.__mode))
        # load the already collected results
        try:
            self.__results = json_from_file(idfile)
        except FileNotFoundError:
            self.__results = {}
        self.__validate()

    def __common_args(self, settings):
        """append arguments common for server and client"""
        args = ['--size=' + str(settings['bs'])]
        if settings['ib_tool'] == 'ib_read_bw':
            args.extend(
                ['--qp=' + str(settings['threads']),
                 '--tx-depth=' + str(settings['iodepth']),
                 '--report_gbits'])
        else:
            args.append('--perform_warm_up')
        return args

    def __server_start(self, settings):
        """Start the server on the remote side (using RemoteCmd)
           and keep an object allowing to control the server.
        """
        print('[size: {}, threads: {}, tx_depth: {}, iters: {}] '\
              '(duration: ~60s)'
              .format(settings['bs'], settings['threads'],
                      settings['iodepth'], settings['iterations']))
        r_numa_n = str(self.__config['REMOTE_JOB_NUMA'])
        r_ib_path = join(self.__config['REMOTE_IB_PATH'], settings['ib_tool'])
        r_aux_params = [*self.__config['REMOTE_AUX_PARAMS'], *settings['args']]

        args = ['numactl', '-N', r_numa_n, r_ib_path, *r_aux_params]
        # XXX add option to dump the command
        self.__server = RemoteCmd.run_async(self.__config, args, None)

    def __server_stop(self, settings):
        """wait until server finishes"""
        self.__server.wait()
        stdout = self.__server.stdout.read().decode().strip()
        stderr = self.__server.stderr.read().decode().strip()
        with open(settings['logfile_server'], 'w', encoding='utf-8') as log:
            log.write('\nstdout:\n{}\nstderr:\n{}\n'.format(stdout, stderr))

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

    def __set_log_files_names(self):
        """set names of log files"""
        time_stamp = datetime.now().strftime("%Y-%m-%d-%H:%M:%S.%f")
        name = '/tmp/ib_read_{}-{}'.format(self.__mode, time_stamp)
        self.__settings['logfile_server'] = name + '-server.log'
        self.__settings['logfile_client'] = name + '-client.log'

    def run(self):
        """run the benchmarking sequence"""
        self.__set_log_files_names()
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
            settings['args'] = self.__common_args(settings)
            # XXX remote_command --pre
            self.__server_start(settings)
            y_value = self.__client_run(settings)
            self.__server_stop(settings)
            # XXX remote_command --post
            self.__result_append(x_value, y_value)

    X_KEYS = ['threads', 'bs', 'iodepth']

    SETTINGS_BY_MODE = {
        'bw-bs': {
            'ib_tool': 'ib_read_bw',
            'threads': 1,
            'bs': BS_VALUES,
            'iodepth': 2,
            # values measured empirically, so that duration was ~60s
            'iterations': {
                256: 48336720,
                1024: 48336720,
                4096: 34951167,
                8192: 24475088,
                16384: 23630690,
                32768: 8299603,
                65536: 5001135,
                131072: 4800000,
                262144: 2600000
            }
        },
        'bw-dp-exp': {
            'ib_tool': 'ib_read_bw',
            'threads': 1,
            'bs': 4096,
            'iodepth': [1, 2, 4, 8, 16, 32, 64, 128],
            # values measured empirically, so that duration was ~60s
            'iterations': {
                1:   20769620,
                2:   30431214,
                4:   45416656,
                8:   65543498,
                16:  85589536,
                32:  100000000,
                64:  100000000,
                128: 100000000
            }
        },
        'bw-dp-lin': {
            'ib_tool': 'ib_read_bw',
            'threads': 1,
            'bs': 4096,
            'iodepth': [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
            # values measured empirically, so that duration was ~60s
            'iterations': {
                1: 20609419,
                2: 30493585,
                3: 40723132,
                4: 43536049,
                5: 50576557,
                6: 55879517,
                7: 60512919,
                8: 65088286,
                9: 67321386,
                10: 68566797
            }
        },
        'bw-th': {
            'ib_tool': 'ib_read_bw',
            'threads': [1, 2, 4, 8, 12],
            'bs': 4096,
            'iodepth': 2,
            # values measured empirically, so that duration was ~60s
            'iterations': {
                1: 16527218,
                2: 32344690,
                4: 61246542,
                8: 89456698,
                12: 89591370
            }
        },
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
            }
        }
    }
