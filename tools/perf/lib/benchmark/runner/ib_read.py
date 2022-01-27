#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# ib_read.py
#

"""the ib_read_{lat,bw} tools runner (EXPERIMENTAL)"""

import subprocess
import time
import shutil
from datetime import datetime
from os.path import join
import lib.format as fmt
from ...common import json_from_file
from ...remote_cmd import RemoteCmd
from .common import UNKNOWN_VALUE_MSG, NO_X_AXIS_MSG, MISSING_KEY_MSG, \
                    BS_VALUES, run_pre_command, run_post_command, \
                    result_append, result_is_done, print_start_message, \
                    verify_oneseries

class IbReadRunner:
    """the ib_read_{lat,bw} tools runner

    The runner executes directly either the `ib_read_lat` or `ib_read_bw` binary
    on both ends of the connection.
    """
    def __validate(self):
        """validate the object and readiness of the env"""
        # check if the local ib tool is present
        if shutil.which(self.__ib_path) is None:
            raise ValueError("cannot find the local ib tool: {}"
                             .format(self.__ib_path))
        # check if the remote ib tool is present
        if 'SERVER_IP' not in self.__config:
            raise ValueError(MISSING_KEY_MSG.format('SERVER_IP'))
        if not self.__skip_remote_cmds:
            output = RemoteCmd.run_sync(self.__config,
                                        ['which', self.__r_ib_path])
            if output.exit_status != 0:
                raise ValueError("cannot find the remote ib tool: {}"
                                 .format(self.__r_ib_path))

    def __init__(self, benchmark, config, idfile):
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        self.__server = None
        # set dumping commands
        self.__dump_cmds = self.__config.get('DEBUG_DUMP_CMDS', False)
        self.__skip_running_tools = \
            self.__config.get('DEBUG_SKIP_RUNNING_TOOLS', False)
        self.__skip_remote_cmds = \
            self.__config.get('DEBUG_SKIP_REMOTE_CMDS', False)
        verify_oneseries(self.__benchmark.oneseries, self.__ONESERIES_REQUIRED)
        # pick the settings predefined for the chosen mode
        self.__tool = self.__benchmark.oneseries['tool']
        self.__mode = self.__benchmark.oneseries['mode']
        self.__settings = self.__SETTINGS_BY_MODE.get(self.__mode, None)
        if not isinstance(self.__settings, dict):
            raise ValueError(UNKNOWN_VALUE_MSG.format('mode', self.__mode))
        # path to the local ib tool
        ib_name = self.__benchmark.oneseries['tool'] + '_' + \
            self.__benchmark.oneseries['tool_mode']
        self.__ib_path = join(self.__config.get('IB_PATH', ''), ib_name)
        # path to the remote ib tool
        self.__r_ib_path = join(self.__config.get('REMOTE_IB_PATH', ''),
                                ib_name)
        # find the x-axis key
        self.__x_key = None
        for x_key in self.__X_KEYS:
            if isinstance(self.__settings.get(x_key), list):
                self.__x_key = x_key
                break
        if self.__x_key is None:
            raise NotImplementedError(NO_X_AXIS_MSG.format(self.__mode))
        # load the already collected results
        try:
            self.__results = json_from_file(idfile)
        except FileNotFoundError:
            self.__results = {'input_file': idfile, 'json': []}
        self.__data = self.__results['json']
        self.__validate()
        self.__formatter = fmt.IbReadLatFormat if self.__mode == 'lat' \
                                               else fmt.IbReadBwFormat

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
        r_aux_params = [*settings['args']]
        cfg_r_aux_params = self.__config.get('REMOTE_AUX_PARAMS', '').split(' ')
        if cfg_r_aux_params != ['']:
            r_aux_params = r_aux_params + cfg_r_aux_params

        args = ['numactl', '-N', r_numa_n, self.__r_ib_path, *r_aux_params]
        # dump a command to the log file
        if self.__dump_cmds:
            with open(settings['logfile_server'], 'a', encoding='utf-8') as log:
                log.write("[server]$ {}".format(' '.join(args)))
        if not (self.__skip_running_tools or self.__skip_remote_cmds):
            self.__server = RemoteCmd.run_async(self.__config, args)
            time.sleep(0.1) # wait 0.1 sec for server to start listening

    def __server_stop(self, settings):
        """wait until server finishes"""
        if self.__skip_running_tools:
            return
        self.__server.wait()
        stdout = self.__server.stdout.read().decode().strip()
        stderr = self.__server.stderr.read().decode().strip()
        with open(settings['logfile_server'], 'a', encoding='utf-8') as log:
            log.write('\nstdout:\n{}\nstderr:\n{}\n'.format(stdout, stderr))

    @staticmethod
    def __probably_no_server(error: subprocess.CalledProcessError) -> bool:
        """If the following error messages were found in the stderr,
           it can indicate that the server has probably not been started yet.
        """
        if 'Unable to init the socket connection' in error.stderr:
            return True
        if 'Unable to perform rdma_client function' in error.stderr:
            return True
        return False

    def __client_run(self, settings):
        """run the client (locally) and wait till the end of execution"""
        numa_n = str(self.__config['JOB_NUMA'])
        if self.__config.get('DEBUG_SHORT_RUNTIME', False):
            it_opt = '--iters=10'
        else:
            it_opt = '--iters=' + str(settings['iterations'])
        aux_params = [*settings['args']]
        cfg_aux_params = self.__config.get('AUX_PARAMS', '').split(' ')
        if cfg_aux_params != ['']:
            aux_params = aux_params + cfg_aux_params
        server_ip = self.__config['SERVER_IP']
        args = ['numactl', '-N', numa_n, self.__ib_path, *aux_params,
                it_opt, server_ip]
        # dump a command to the log file
        if self.__dump_cmds:
            with open(settings['logfile_client'], 'a', encoding='utf-8') as log:
                log.write("[client]$ {}".format(' '.join(args)))

        if not self.__skip_running_tools:
            # XXX optionally measure the run time and assert exe_time >= 60s
            # try to connect with the server 10 times at most
            counter = 1
            while True:
                try:
                    ret = subprocess.run(args, check=True,
                                         stdout=subprocess.PIPE,
                                         stderr=subprocess.PIPE,
                                         encoding='utf-8')
                    break
                except subprocess.CalledProcessError as err:
                    if not self.__probably_no_server(err) or counter == 10:
                        print('\nstdout:\n{}\nstderr:\n{}\n'
                              .format(err.stdout, err.stderr))
                        self.__server_stop(settings)
                        run_post_command(self.__config,
                                         self.__benchmark.oneseries)
                        raise # re-raise the current exception
                    print('Retrying #{} ...'.format(counter))
                    time.sleep(0.1) # wait 0.1 sec for server to start listening
                    counter = counter + 1

            # save stderr in the log file
            with open(settings['logfile_client'], 'a', encoding='utf-8') as log:
                log.write('\nstderr:\n{}\n'.format(ret.stderr))
            result = self.__formatter.parse(ret.stdout,
                                            str(settings['bs']),
                                            settings['threads'],
                                            settings['iodepth'])
        else:
            result = self.__formatter.random_results(settings['bs'],
                                            settings['threads'],
                                            settings['iodepth'])

        return result

    def __result_append(self, _, y_value: dict):
        """append new result to internal __data and the '__idfile' file"""
        result_append(self.__data, self.__idfile, y_value)

    def __result_is_done(self, x_value: int):
        """check if the result for the given x value is already collected"""
        return result_is_done(self.__data, self.__x_key, x_value)

    def __set_log_files_names(self):
        """set names of log files"""
        time_stamp = datetime.now().strftime("%Y-%m-%d-%H:%M:%S.%f")
        name = '/tmp/{}_{}-{}'.format(self.__tool, self.__mode, time_stamp)
        self.__settings['logfile_server'] = name + '-server.log'
        self.__settings['logfile_client'] = name + '-client.log'
        print('Server log: {}'.format(self.__settings['logfile_server']))
        print('Client log: {}'.format(self.__settings['logfile_client']))

    def run(self):
        """collects the `benchmark` results using `ib_read_lat` or `ib_read_bw`

        For each of the x values:

        1. starts the `ib_read_*` server on the remote side.
        2. starts and waits for the `ib_read_*` client to the end.
            - the results are collected and written to the `idfile` file.
        3. stops the `ib_read_*` server on the remote side.

        Raises:
            NotImplementedError: If the number of iterations for a given x value
              is not set. `--iters` is used instead of `--duration` because
              the latter produces significantly less detailed output.
        """
        print_start_message(self.__mode, self.__benchmark.oneseries,
                            self.__config)
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
            pre_cmd = run_pre_command(self.__config,
                                      self.__benchmark.oneseries, x_value)
            self.__server_start(settings)
            y_value = self.__client_run(settings)
            self.__server_stop(settings)
            run_post_command(self.__config, self.__benchmark.oneseries, pre_cmd)
            self.__result_append(x_value, y_value)

    __ONESERIES_REQUIRED = {
        'tool': ['ib_read'],
        'tool_mode': ['lat', 'bw'],
        'mode': ['lat', 'bw-bs', 'bw-dp-exp', 'bw-dp-lin', 'bw-th'],
        'rw': ['read'],
        'filetype': ['malloc']
    }

    __X_KEYS = ['threads', 'bs', 'iodepth']

    __SETTINGS_BY_MODE = {
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
