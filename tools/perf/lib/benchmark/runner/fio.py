#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# fio.py
#

"""the FIO runner (EXPERIMENTAL)"""

import time
import subprocess
from datetime import datetime
from os.path import join
from shutil import which
from lib.format import FioFormat
from ...common import json_from_file
from ...remote_cmd import RemoteCmd
from .common import UNKNOWN_MODE_MSG, NO_X_AXIS_MSG, BS_VALUES, \
                    result_append, result_is_done, print_start_message, \
                    run_pre_command, run_post_command

UNKNOWN_RW_MSG = "An unexpected 'rw' value: {}"
UNKNOWN_FILETYPE_MSG = "An unexpected 'filetype' value: {}"

class FioRunner:
    """the FIO runner

    The runner executes directly the `fio` binary on both ends of
    the connection.
    """

    def __validate(self):
        """validate the object and readiness of the env"""
        # XXX validate the object
        filetype = self.__benchmark.oneseries['filetype']
        if filetype not in ['malloc', 'pmem']:
            raise ValueError(UNKNOWN_FILETYPE_MSG.format(filetype))

        # check if the local fio is present
        if which(self.__fio_path) is None:
            raise ValueError("cannot find the local fio: {}"
                             .format(self.__fio_path))

        # check if the remote fio is present
        output = RemoteCmd.run_sync(self.__config, ['which', self.__r_fio_path])
        if output.exit_status != 0:
            raise ValueError("cannot find the remote fio: {}"
                             .format(self.__r_fio_path))

    def __set_settings_by_mode(self):
        """set all variable elements of __SETTINGS_BY_MODE"""
        # set 'threads' to CORES_PER_SOCKET in the 'bw-cpu-mt' mode
        if self.__mode == 'bw-cpu-mt':
            self.__settings['threads'] = self.__config['CORES_PER_SOCKET']
        # set values of 'cpuload' and their 'iterations':
        if 'cpu' in self.__mode:
            if 'cpu_load_range' in self.__benchmark.oneseries:
                cpu_load = self.__benchmark.oneseries['cpu_load_range']
            else:
                cpu_load = '00_99'
            if cpu_load not in self.__CPU_LOAD_RANGE:
                raise ValueError('wrong value of \'cpu_load_range\': {}'
                                 .format(cpu_load))
            cpu_load_range = self.__CPU_LOAD_RANGE[cpu_load]
            self.__settings['cpuload'] = cpu_load_range
            self.__settings['iterations'] = len(cpu_load_range)

    def __init__(self, benchmark, config: dict, idfile: str) -> 'FioRunner':
        # XXX nice to have REMOTE_JOB_NUMA_CPULIST, CORES_PER_SOCKET
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        self.__server = None
        self.__dump_cmds = False
        # set dumping commands
        if 'DUMP_CMDS' in self.__config and self.__config['DUMP_CMDS']:
            self.__dump_cmds = True
        # pick the result keys base on the benchmark's rw
        readwrite = benchmark.oneseries['rw']
        if 'read' in readwrite:
            self.__result_keys = ['read']
        elif 'write' in readwrite:
            self.__result_keys = ['write']
        elif 'rw' in readwrite:
            self.__result_keys = ['read', 'write']
        else:
            raise ValueError(UNKNOWN_RW_MSG.format(readwrite))
        # pick the settings predefined for the chosen mode
        self.__tool = self.__benchmark.oneseries['tool']
        self.__tool_mode = self.__benchmark.oneseries['tool_mode']
        self.__mode = self.__benchmark.oneseries['mode']
        self.__direct_write_to_pmem = \
            int(self.__benchmark.requirements['direct_write_to_pmem'])
        self.__settings = self.__SETTINGS_BY_MODE.get(self.__mode, None)
        if not isinstance(self.__settings, dict):
            raise NotImplementedError(UNKNOWN_MODE_MSG.format(self.__mode))
        self.__set_settings_by_mode()
        # path to the local fio
        self.__fio_path = join(self.__config.get('FIO_PATH', ''), 'fio')
        # path to the remote fio
        self.__r_fio_path = join(self.__config.get('REMOTE_FIO_PATH', ''),
                                 'fio')
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

    def __server_start(self, settings):
        """Start the server on the remote side (using RemoteCmd)
           and keep an object allowing to control the server.
        """
        operation = self.__benchmark.oneseries['rw']
        if 'rw_dir' in self.__benchmark.oneseries:
            operation = operation + '-' + self.__benchmark.oneseries['rw_dir']
        if 'cpuload' in settings:
            cpuload = settings['cpuload']
        else:
            cpuload = 0
        print('[mode: {}, op: {}, size: {}, threads: {}, tx_depth: {}, '\
              'sync: {}, cpuload: {}]'\
              .format(self.__tool_mode, operation, settings['bs'],
                      settings['threads'], settings['iodepth'],
                      settings['sync'], cpuload))
        r_numa_n = str(self.__config['REMOTE_JOB_NUMA'])
        # XXX nice to have REMOTE_TRACER
        args = ['numactl', '-N', r_numa_n, self.__r_fio_path]
        busy_wait_polling = \
            int(self.__benchmark.oneseries.get('busy_wait_polling', True))
        env = ['serverip={}'.format(self.__config['server_ip']),
               'numjobs={}'.format(settings['threads']),
               'iodepth={}'.format(settings['iodepth']),
               'direct_write_to_pmem={}'.format(self.__direct_write_to_pmem),
               'busy_wait_polling={}'.format(busy_wait_polling),
               'cores_per_socket={}'.format(self.__config['CORES_PER_SOCKET'])]
        if 'cpuload' in settings and settings['cpuload'] > 0:
            env.append('cpuload={}'.format(settings['cpuload']))
        else:
            # no CPU load
            args.append('--section=server')
        # XXX the name REMOTE_JOB_MEM_PATH is unfortunate since for
        # rpma_fio_bench.sh it is meant to store also a possible 'malloc'
        # value
        # XXX pmem_path accommodates both DeviceDAX and FileSystemDAX where
        # the config may have both provided in the future allowing to pick
        # either one or another here and generate a figure comparing both
        # PMem modes.
        pmem_path = self.__config['REMOTE_JOB_MEM_PATH']
        # pick either a DRAM, DeviceDAX or a FileSystemDAX
        if self.__benchmark.oneseries['filetype'] == 'malloc':
            # create_on_open prevents FIO from creating files
            # where the engines won't make use of them anyways
            # since they are using DRAM instead
            args.extend([
                '--filename=malloc', '--create_on_open=1'])
        elif '/dev/dax' in pmem_path:
            # DeviceDAX
            args.append('--filename={}'.format(pmem_path))
        else:
            # FileSystemDAX
            # XXX make sure the '$jobnum' is properly escaped and hints fio
            # to replace the variable with a correct value.
            args.append(
                '--filename_format={}.\\$jobnum'.format(pmem_path))
        # copy the job file to the server
        r_job_path = self.__config.get('REMOTE_JOB_PATH', '')
        if r_job_path == '':
            r_job_path = '/dev/shm/librpma_{}-server.fio'\
                .format(self.__tool_mode)
        job_file = "./fio_jobs/librpma_{}-server.fio".format(self.__tool_mode)
        RemoteCmd.copy_to_remote(self.__config, job_file, r_job_path)
        args.append(r_job_path)
        args = env + args
        # dump a command to the log file
        if self.__dump_cmds:
            with open(settings['logfile_server'], 'a', encoding='utf-8') as log:
                log.write("[server]$ {}".format(' '.join(args)))
        self.__server = RemoteCmd.run_async(self.__config, args)
        time.sleep(0.1) # wait 0.1 sec for server to start listening

    def __server_stop(self, settings):
        """wait until server finishes"""
        self.__server.wait()
        stdout = self.__server.stdout.read().decode().strip()
        stderr = self.__server.stderr.read().decode().strip()
        with open(settings['logfile_server'], 'a', encoding='utf-8') as log:
            log.write('\nstdout:\n{}\nstderr:\n{}\n'.format(stdout, stderr))

    def __client_run(self, settings):
        """run the client (locally) and wait till the end of execution"""
        short_runtime = self.__config.get('SHORT_RUNTIME', False)
        duration = self.__DURATION['short' if short_runtime else 'full']
        env = {
            'serverip': self.__config['server_ip'],
            'numjobs': str(settings['threads']),
            'iodepth': str(settings['iodepth']),
            'blocksize': str(settings['bs']),
            'sync': str(int(settings['sync'])),
            'readwrite': self.__benchmark.oneseries['rw'],
            'ramp_time': str(duration['ramp']),
            'runtime': str(duration['run'])
        }

        if 'TRACER' in self.__config and self.__config['TRACER'] != '':
            args = str(self.__config['TRACER']).split(' ')
        else:
            args = ['numactl', '-N', str(self.__config['JOB_NUMA'])]

        job_file = './fio_jobs/librpma_{}-client.fio'.format(self.__tool_mode)
        args.extend([self.__fio_path, job_file, '--output-format=json+'])

        # dump a command to the log file
        if self.__dump_cmds:
            with open(settings['logfile_client'], 'a', encoding='utf-8') as log:
                log.write("[client]$ {}".format(' '.join(args)))
        try:
            ret = subprocess.run(args, check=True, env=env,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 encoding='utf-8')
        except subprocess.CalledProcessError as err:
            print('\nstdout:\n{}\nstderr:\n{}\n'
                  .format(err.stdout, err.stderr))
            self.__server_stop(settings)
            run_post_command(self.__config, self.__benchmark.oneseries)
            raise # re-raise the current exception

        result = FioFormat.parse(ret.stdout)

        # append cpuload to the results
        if 'cpuload' in settings:
            for key in self.__result_keys:
                result[key]['cpuload'] = settings['cpuload']

        # return the measured value
        # in case of a mixed workload the result is a tuple
        if len(self.__result_keys) == 1:
            ret_val = result[self.__result_keys[0]]
        else:
            ret_val = (result[self.__result_keys[0]],
                       result[self.__result_keys[1]])
        return ret_val

    def __result_append(self, _, y_value: dict):
        """append new result to internal __data and the '__idfile' file"""
        result_append(self.__data, self.__idfile, y_value)

    def __result_is_done(self, x_value: int):
        """check if the result for the given x value is already collected"""
        return result_is_done(self.__data, self.__x_key, x_value)

    def __set_log_files_names(self):
        """set names of log files"""
        time_stamp = datetime.now().strftime("%Y-%m-%d-%H:%M:%S.%f")
        name = '/tmp/{}_{}_{}-{}'.format(self.__tool, self.__tool_mode,
                                         self.__mode, time_stamp)
        self.__settings['logfile_server'] = name + '-server.log'
        self.__settings['logfile_client'] = name + '-client.log'
        print('Server log: {}'.format(self.__settings['logfile_server']))
        print('Client log: {}'.format(self.__settings['logfile_client']))

    def run(self) -> None:
        """collects the `benchmark` results using `fio`

        For each of the x values:

        1. starts the `fio` server on the remote side.
        2. starts and waits for the `fio` client to the end.
            - the results are collected and written to the `idfile` file.
        3. stops the `fio` server on the remote side.
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
            pre_cmd = run_pre_command(self.__config,
                                      self.__benchmark.oneseries, x_value)
            self.__server_start(settings)
            y_value = self.__client_run(settings)
            self.__server_stop(settings)
            run_post_command(self.__config, self.__benchmark.oneseries, pre_cmd)
            self.__result_append(x_value, y_value)

    __CPU_LOAD_RANGE = {
        '00_99' : [0, 25, 50, 75, 99],
        '75_99' : [75, 80, 85, 90, 95, 99]
    }

    __DURATION = {
        'short': {
            'ramp': 0,
            'run': 10,
        },
        'full': {
            'ramp': 15,
            'run': 60
        }
    }

    __X_KEYS = ['threads', 'bs', 'iodepth', 'cpuload']
    __BW_DP_EXP_DEPTHS = [1, 2, 4, 8, 16, 32, 64, 128]
    __BW_DP_LIN_DEPTHS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    __THREADS_VALUES = [1, 2, 4, 8, 12, 16, 32, 64]

    __SETTINGS_BY_MODE = {
        'bw-bs': {
            'threads': 1,
            'bs': BS_VALUES,
            'iterations': len(BS_VALUES),
            'iodepth': 2,
            'sync': False
            },
        'bw-dp-exp': {
            'threads': 1,
            'bs': 4096,
            'iodepth': __BW_DP_EXP_DEPTHS,
            'iterations': len(__BW_DP_EXP_DEPTHS),
            'sync': False
            },
        'bw-dp-lin': {
            'threads': 1,
            'bs': 4096,
            'iodepth': __BW_DP_LIN_DEPTHS,
            'iterations': len(__BW_DP_LIN_DEPTHS),
            'sync': False
            },
        'bw-th': {
            'threads': __THREADS_VALUES,
            'bs': 4096,
            'iodepth': 2,
            'iterations': len(__THREADS_VALUES),
            'sync': False
            },
        'bw-cpu': {
            'threads': 1,
            'bs': 65536,
            'iodepth': 2,
            'cpuload': [], # will be set in __init__
            'iterations': 0, # will be set in __init__
            'sync': False
            },
        'bw-cpu-mt': {
            'threads': 0, # will be set to CORES_PER_SOCKET in __init__
            'bs': 4096,
            'iodepth': 2,
            'cpuload': [], # will be set in __init__
            'iterations': 0, # will be set in __init__
            'sync': False
            },
        'lat': {
            'threads': 1,
            'bs': BS_VALUES,
            'iodepth': 1,
            'iterations': len(BS_VALUES),
            'sync': True
            },
        'lat-cpu': {
            'threads': 1,
            'bs': 4096,
            'iodepth': 1,
            'cpuload': [], # will be set in __init__
            'iterations': 0, # will be set in __init__
            'sync': True
            }
    }
