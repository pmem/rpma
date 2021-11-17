#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""ib_read.py -- the ib_read_{lat,bw} tools runner (EXPERIMENTAL)"""

from ...common import json_from_file
# XXX from .common import RemoteCmd

NO_X_AXIS_MSG = \
    "SETTINGS_BY_MODE[{}] is missing a key defined as a list (x-axis)"

class IbReadRunner:
    """XXX"""

    ONESERIES_REQUIRED = {
        'rw': 'read',
        'filetype': 'malloc'
    }

    def __validate(self):
        """XXX"""
        for key, value in self.ONESERIES_REQUIRED.items():
            if self.__benchmark.oneseries[key] != value:
                present_value = self.__benchmark.oneseries[key]
                raise ValueError(".{} == {} != {}".format(key, present_value,
                                                          value))
        # XXX check if self.__settings['ib_tool'] is present on both sides

    def __init__(self, benchmark, config, idfile):
        """XXX"""
        # XXX REQUIRED_REMOTE_DIRECT_WRITE_TO_PMEM=0
        # XXX set_ddio /dev/shm
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        # pick the settings predefined for the chosen mode
        mode = self.__benchmark.oneseries['mode']
        self.__settings = self.SETTINGS_BY_MODE.get(mode, None)
        if not isinstance(self.__settings, dict):
            raise NotImplementedError(
                "SETTINGS_BY_MODE[{}] is missing".format(mode))
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

    def __server_start(self, _settings):
        # XXX
        pass

    def __server_stop(self, _settings):
        # XXX
        pass

    def __client_run(self, _settings):
        # XXX
        return 0 # XXX

    def __result_append(self, x_value, y_value):
        # XXX this part is probably common betwen ib_read and fio classes
        # maybe it should fit into the BaseRunner class? TBD
        # I think it should also be step-by-step cached into a file
        # so the execution process will be more resilient
        pass

    def __result_is_done(self, x_value):
        """XXX"""

    def run(self):
        """XXX"""
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
            'bs': [256, 1024, 4096, 8192, 16384, 32768, 65536, 131072, 262144],
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
                262144: 2100000},
		    'aux_params': ['--perform_warm_up']}
        # XXX TBD
    }
