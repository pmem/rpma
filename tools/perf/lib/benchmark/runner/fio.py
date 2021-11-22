#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""fio.py -- the FIO runner (EXPERIMENTAL)"""

from os.path import exists
from ...common import json_from_file
from .common import UNKNOWN_MODE_MSG, NO_X_AXIS_MSG, BS_VALUES # XXX RemoteCmd

UNKNOWN_RW_MSG = "An unexpected 'rw' value: {}"
UNKNOWN_FILETYPE_MSG = "An unexpected 'filetype' value: {}"

class FioRunner:
    """the FIO runner"""

    def __validate(self):
        """validate the object and readiness of the env"""
        # XXX validate the object
        filetype = self.__benchmark.oneseries['filetype']
        if filetype not in ['malloc', 'pmem']:
            raise ValueError(UNKNOWN_FILETYPE_MSG.format(filetype))

        fio_local_path = str(self.__config.get('FIO_PATH', "")) + "fio"
        if not exists(fio_local_path):
            raise ValueError("the local fio does not exist: {}"
                             .format(fio_local_path))
        # XXX check if:
        # - ${REMOTE_FIO_PATH}fio present remotely (using RemoteCmd)

    def __init__(self, benchmark, config, idfile):
        """create the object"""
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
        # keep an object allowing to control the server on the remote side
        _unused = args

    def __server_stop(self, _settings):
        # XXX check the server is stopped
        pass

    TIME = {
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
        time = self.TIME['short' if short_runtime else 'full']
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

    def run(self):
        """run the benchmarking sequence"""
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

    X_KEYS = ['threads', 'bs', 'iodepth']

    SETTINGS_BY_MODE = {
        'lat': {
            'threads': 1,
            'bs': BS_VALUES,
            'iodepth': 1,
            'sync': True}
    }
    # XXX TBD
