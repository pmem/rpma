#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# common.py
#

"""the runner's helpers (EXPERIMENTAL)"""

import json
import re

from ...remote_cmd import RemoteCmd

#: an error message when an unexpected value of a key is detected
UNKNOWN_VALUE_MSG = "An unexpected '{}' value: {}"

#: an error message when x-axis cannot be identified
NO_X_AXIS_MSG = \
    "SETTINGS_BY_MODE[{}] is missing a key defined as a list (x-axis)"

#: an error message when a key is missing in the figure
MISSING_KEY_MSG = "the following key is missing in the figure: {}"

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
          .format(tool, mode, config['SERVER_IP']))

def prepare_cmd(config, oneseries, cmd_exec, x_value=None):
    """prepare cmd"""
    cmd = cmd_exec
    cmd_vars = re.findall(r'\${.+?}', cmd)
    for item in cmd_vars:
        replace_item = item.replace('${', '').replace('}', '')
        if replace_item in config:
            cmd = cmd.replace(item, str(config[replace_item]))
        elif replace_item == 'RUN_NAME' and x_value is not None:
            run_name = 'benchmark_{}_x_value_{}'\
                .format(oneseries['id'], x_value)
            cmd = cmd.replace(item, run_name)
    return cmd

def run_pre_command(config, oneseries, x_value):
    """run pre command"""
    run_pre = None
    if 'REMOTE_CMD_PRE' in config and config['REMOTE_CMD_PRE'] != '':
        cmd = prepare_cmd(config, oneseries, config['REMOTE_CMD_PRE'], x_value)
        run_pre = RemoteCmd.run_async(config, cmd)
    return run_pre

def __wait_for_pre_command(pre_command, raise_on_error=True):
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

def run_post_command(config, oneseries, pre_command=None):
    """run post command and wait for the end of the pre command"""
    if 'REMOTE_CMD_POST' in config and config['REMOTE_CMD_POST'] != '':
        cmd = prepare_cmd(config, oneseries, config['REMOTE_CMD_POST'])
        try:
            RemoteCmd.run_sync(config, cmd, raise_on_error=True)
        except:
            print('Post-command failed!')
            __wait_for_pre_command(pre_command, raise_on_error=False)
            print('--- post-command\'s output: ---')
            raise
    __wait_for_pre_command(pre_command)

def verify_oneseries(actual, required):
    """verify if oneseries contains all required keys
       and if they have required/allowed values
    """
    for key, values in required.items():
        if key not in actual:
            raise ValueError(MISSING_KEY_MSG.format(key))
        if values is None:
            continue
        if not isinstance(values, list):
            raise ValueError('value of the {} key has to be a list'
                             .format(key))
        actual_value = actual[key]
        if actual_value not in values:
            raise ValueError('.{} == {} not in {}'
                             .format(key, actual_value, values))
