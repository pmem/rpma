#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# fio.py
#

"""the FIO runner (EXPERIMENTAL)"""

from os.path import join
from shutil import which
from ...common import json_from_file
from ...remote_cmd import RemoteCmd
from .common import UNKNOWN_MODE_MSG, NO_X_AXIS_MSG, BS_VALUES, \
                    CPU_LOAD_00_99, THREADS_VALUES

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

        fio_local_path = join(self.__config.get('FIO_PATH', ''), 'fio')
        if which(fio_local_path) is None:
            raise ValueError("cannot find the local fio: {}"
                             .format(fio_local_path))

        fio_remote_path = join(self.__config.get('REMOTE_FIO_PATH', ''), 'fio')
        output = RemoteCmd.run_sync(self.__config, ['which', fio_remote_path])
        if output.exit_status != 0:
            raise ValueError("cannot find the remote fio: {}"
                             .format(fio_local_path))

    def __init__(self, benchmark, config: dict, idfile: str) -> 'FioRunner':
        # XXX nice to have REMOTE_JOB_NUMA_CPULIST, CORES_PER_SOCKET
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
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
        mode = self.__benchmark.oneseries['mode']
        self.__settings = self.__SETTINGS_BY_MODE.get(mode, None)
        if not isinstance(self.__settings, dict):
            raise NotImplementedError(UNKNOWN_MODE_MSG.format(mode))
        # find the x-axis key
        self.__x_key = None
        for x_key in self.__X_KEYS:
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
        if mode == 'bw-cpu-mt':
            self.__settings['threads'] = config['CORES_PER_SOCKET']

    def __server_start(self, settings):
        # XXX to be removed when the implementation will be complete
        # pylint: disable=unused-variable
        args = []
        env = {
            'serverip': self.__config['server_ip'],
            'numjobs': settings['threads'],
            'iodepth': settings['iodepth'],
            'direct_write_to_pmem': False, # XXX
            'busy_wait_polling':
                self.__benchmark.oneseries.get('busy_wait_polling', True),
            # XXX CORES_PER_SOCKET may be a new required config.json argument
            # or has to be read from the remote system.
            'cores_per_socket': self.__config['CORES_PER_SOCKET']
        }
        if 'cpu' in settings.keys():
            env['cpuload'] = settings['cpu']
        else:
            # no CPU load
            args.append('--section=server')
        # XXX the name REMOTE_JOB_MEM_PATH is unfortunate since for
        # rpma_fio_bench.sh it is meant to store also a possible 'malloc'
        # value
        # XXX pmem_path accomodates both DeviceDAX and FileSystemDAX where
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
        # XXX nice to have REMOTE_TRACER
        # XXX copy the job file to the server
        # XXX start a server on the remote side (using RemoteCmd)
        # XXX add option to dump the command (DUMP_CMDS)
        # keep an object allowing to control the server on the remote side
        _unused = args

    def __server_stop(self, _settings):
        # XXX check the server is stopped
        pass

    __TIME = {
        'short': {
            'ramp': 0,
            'run': 10,
        },
        'full': {
            'ramp': 15,
            'run': 60
        }
    }

    def __client_run(self, settings):
        # pylint: disable=unused-variable
        short_runtime = self.__config.get('SHORT_RUNTIME', False)
        time = self.__TIME['short' if short_runtime else 'full']
        args = []
        env = {
            'serverip': self.__config['server_ip'],
            'numjobs': settings['threads'],
            'iodepth': settings['iodepth'],
            'blocksize': settings['bs'],
            'sync': settings['sync'],
            'readwrite': self.__benchmark.oneseries['rw'],
            'ramp_time': time['ramp'],
            'runtime': time['run']
        }
        # XXX run the client (locally) and wait till the end of execution
        # XXX add option to dump the command (DUMP_CMDS)
        # XXX convert the fio_json2csv.py script into a module?
        # XXX return the measured value (Note: self.__result_keys)
        # XXX convert the ./csv2standardized.py script into a module?
        # in case of a mixed workload the result is a tuple
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

    def run(self) -> None:
        """collects the `benchmark` results using `fio`

        For each of the x values:

        1. starts the `fio` server on the remote side.
        2. starts and waits for the `fio` client to the end.
            - the results are collected and written to the `idfile` file.
        3. stops the `fio` server on the remote side.
        """
        # benchmarks are run for all x values one-by-one
        for x_value in self.__settings[self.__x_key]:
            if self.__result_is_done(x_value):
                continue
            # prepare settings for the current x-axis value
            settings = self.__settings.copy()
            settings[self.__x_key] = x_value
            # XXX remote_command --pre
            self.__server_start(settings)
            y_value = self.__client_run(settings)
            self.__server_stop(settings)
            # XXX remote_command --post
            self.__result_append(x_value, y_value)

    __X_KEYS = ['threads', 'bs', 'iodepth']

    __BW_DP_EXP_DEPTHS = [1, 2, 4, 8, 16, 32, 64, 128]
    __BW_DP_LIN_DEPTHS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

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
            'threads': THREADS_VALUES,
            'bs': 4096,
            'iodepth': 2,
            'iterations': len(THREADS_VALUES),
            'sync': False
            },
        'bw-cpu': {
            'threads': 1,
            'bs': 65536,
            'iodepth': 2,
            'cpuload': CPU_LOAD_00_99,
            'iterations': len(CPU_LOAD_00_99),
            'sync': False
            },
        'bw-cpu-mt': {
            'threads': 0,
            'bs': 4096,
            'iodepth': 2,
            'cpuload': CPU_LOAD_00_99,
            'iterations': len(CPU_LOAD_00_99),
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
            'cpuload': CPU_LOAD_00_99,
            'iterations': len(CPU_LOAD_00_99),
            'sync': True
            }
    }
