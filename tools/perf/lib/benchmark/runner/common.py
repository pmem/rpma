#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# common.py
#

"""the runner's helpers (EXPERIMENTAL)"""

import json
import re

from ...remote_cmd import RemoteCmd

#: an error message when an unexpected mode is detected
UNKNOWN_MODE_MSG = "An unexpected 'mode' value: {}"

#: an error message when x-axis cannot be identified
NO_X_AXIS_MSG = \
    "SETTINGS_BY_MODE[{}] is missing a key defined as a list (x-axis)"

#: a common block sizes list
BS_VALUES = [256, 1024, 4096, 8192, 16384, 32768, 65536, 131072, 262144]

def result_append(data: list, idfile: str, result: dict) -> None:
    """append new result to internal data and file"""
    data.append(result)
    with open(idfile, 'w', encoding='utf-8') as file:
        json.dump(data, file, indent=4)

def result_is_done(data: list, x_key: str, x_value: int) -> bool:
    """check if the result for the given x_value of x_key is already collected"""
    for result in data:
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

def print_start_message(mode, oneseries, config):
    """print the STARTING message"""
    tool = oneseries['tool']
    if 'tool_mode' in oneseries:
        tool = tool + '({})'.format(oneseries['tool_mode'])
    print('STARTING benchmark TOOL={} for MODE={} (IP={}) ...'
          .format(tool, mode, config['server_ip']))

def prepare_cmd(config, oneseries, cmd_exec, x_value = None):
    """prepare cmd"""
    cmd = cmd_exec
    cmd_vars = re.findall(r'\${.+?}', cmd)
    for item in cmd_vars:
        replace_item = item.replace('${', '').replace('}', '')
        if replace_item in config:
            cmd = cmd.replace(item, str(config[replace_item]))
        elif replace_item == 'RUN_NAME' and x_value != '':
            run_name = 'benchmark_{}_x_value_{}'\
                .format(oneseries['id'], x_value)
            cmd = cmd.replace(item, run_name)
    return cmd

def run_pre_command(config, oneseries, x_value):
    """run pre command"""
    if 'REMOTE_CMD_PRE' in config and config['REMOTE_CMD_PRE'] != '':
        cmd = prepare_cmd(config, oneseries, config['REMOTE_CMD_PRE'], x_value)
        cmd = cmd + " &"
        run_pre = RemoteCmd.run_sync(config, cmd, raise_on_error=True)
        # by '&' exit_status is always 0
        if run_pre.stderr:
            raise ValueError(run_pre.stderr)

def run_post_command(config, oneseries):
    """run post command"""
    if 'REMOTE_CMD_POST' in config and config['REMOTE_CMD_POST'] != '':
        cmd = prepare_cmd(config, oneseries, config['REMOTE_CMD_POST'])
        RemoteCmd.run_sync(config, cmd, raise_on_error=True)
