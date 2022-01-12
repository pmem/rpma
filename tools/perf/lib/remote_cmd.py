#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# remote_cmd.py
#

"""running commands on remote node over SSH (EXPERIMENTAL)"""

from typing import Union
import paramiko as para
from scp import SCPClient

class RemoteCmd:
    """run a command on a remote node over SSH"""

    @property
    def stdout(self):
        """stdout of the executing command"""
        return self.__stdout

    @property
    def stderr(self):
        """stderr of the executing command"""
        return self.__stderr

    @property
    def exit_status(self):
        """exit status from the process on the server"""
        return self.__exit_status

    def __init__(self, ssh_client, stdout, stderr, exit_status=None):
        self.__ssh_client = ssh_client
        self.__stdout = stdout
        self.__stderr = stderr
        self.__exit_status = exit_status

    def wait(self) -> None:
        """wait for the command to finish"""
        self.__exit_status = self.__stdout.channel.recv_exit_status()
        self.__ssh_client.close()
        self.__ssh_client = None

    @classmethod
    def __connect_to_host(cls, config: dict) -> para.SSHClient:
        """connect to remote host"""
        kwargs = {}
        kwargs['username'] = config.get('REMOTE_USER', None)
        kwargs['password'] = config.get('REMOTE_PASS', None)

        ssh_client = para.SSHClient()
        ssh_client.load_system_host_keys()
        ssh_client.set_missing_host_key_policy(para.RejectPolicy())
        ssh_client.connect(hostname=config['SERVER_IP'], **kwargs)
        return ssh_client

    @classmethod
    def copy_to_remote(cls, config: dict, files: Union[list, str],
                       remote_dir: str) -> None:
        """copy file(s) to remote host

        Args:
            config: the configuration of the benchmarking system
            files: path of a file to copy or list of paths
            remote_dir: remote directory path
        """
        ssh_client = cls.__connect_to_host(config)

        scp = SCPClient(ssh_client.get_transport())
        scp.put(files, remote_dir)
        scp.close()
        ssh_client.close()

    @classmethod
    def run_sync(cls, config: dict, cmd: Union[list, str], env: dict = None,
                 raise_on_error: bool = False) -> 'RemoteCmd':
        """run a remote command and wait till it is done

        Calls `RemoteCmd.run_async()` + `RemoteCmd.wait()`.

        Args:
            config: the configuration of the benchmarking system
            cmd: a command to run on a remote node
            env: an environment for the command
            raise_on_error: raise an error when `exit_status` is nonzero

        Returns:
            A `RemoteCmd` object allowing to read the results of execution.
        """
        remote_cmd = cls.run_async(config, cmd, env)
        remote_cmd.wait()
        if raise_on_error:
            if remote_cmd.exit_status != 0:
                raise ValueError(remote_cmd.stderr.read().decode().strip())
        return remote_cmd

    @classmethod
    def run_async(cls, config: dict, cmd: Union[list, str],
                  env: dict = None) -> 'RemoteCmd':
        """run a remote command and return a control object

        Args:
            config: the configuration of the benchmarking system
            cmd: a command to run on a remote node
            env: an environment for the command

        Returns:
            A `RemoteCmd` object allowing to wait for the command to end
            (`RemoteCmd.wait()`) and read the results of execution.
        """
        ssh_client = cls.__connect_to_host(config)
        if isinstance(cmd, list):
            cmd = ' '.join(cmd)
        _, stdout, stderr = ssh_client.exec_command(cmd, environment=env)

        return cls(ssh_client, stdout, stderr)
