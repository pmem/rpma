#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""remote_cmd.py -- running commands on remote node over SSH (EXPERIMENTAL)"""

import paramiko as para
from scp import SCPClient

class RemoteCmd:
    """run a command on a remote node over SSH"""

    def __init__(self, ssh_client, stdout, stderr):
        self.ssh_client = ssh_client
        self.stdout = stdout
        self.stderr = stderr
        self.exit_status = None

    def wait(self):
        """wait for the command to finish"""
        self.exit_status = self.stdout.channel.recv_exit_status()
        self.ssh_client.close()
        self.ssh_client = None

    @classmethod
    def __connect_to_host(cls, config):
        """connect to remote host"""
        kwargs = {}
        kwargs['username'] = config.get('REMOTE_USER', None)
        kwargs['password'] = config.get('REMOTE_PASS', None)

        ssh_client = para.SSHClient()
        ssh_client.load_system_host_keys()
        ssh_client.set_missing_host_key_policy(para.RejectPolicy())
        ssh_client.connect(hostname=config['server_ip'], **kwargs)
        return ssh_client

    @classmethod
    def copy_to_remote(cls, config, files, remote_dir):
        """copy file to remote host"""
        ssh_client = cls.__connect_to_host(config)

        scp = SCPClient(ssh_client.get_transport())
        scp.put(files, remote_dir)
        scp.close()
        ssh_client.close()

    @classmethod
    def run_sync(cls, config, cmd, env=None, raise_on_error=False):
        """run a remote command and wait till it is done"""
        remote_cmd = cls.run_async(config, cmd, env, raise_on_error)
        remote_cmd.wait()
        return remote_cmd

    @classmethod
    def run_async(cls, config, cmd, env=None, raise_on_error=False):
        """run a remote command and return a control object"""
        ssh_client = cls.__connect_to_host(config)
        if isinstance(cmd, list):
            cmd = ' '.join(cmd)
        _, stdout, stderr = ssh_client.exec_command(cmd, environment=env)

        if raise_on_error:
            if stdout.channel.recv_exit_status() != 0:
                raise ValueError(stderr.read())

        return cls(ssh_client, stdout, stderr)
