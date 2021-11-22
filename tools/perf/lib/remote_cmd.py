#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""remote_cmd.py -- running commands on remote node over SSH (EXPERIMENTAL)"""

import paramiko as para

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
    def run_sync(cls, config, cmd, env):
        """run a remote command and wait till it is done"""
        remote_cmd = cls.run_async(config, cmd, env)
        remote_cmd.wait()
        return remote_cmd

    @classmethod
    def run_async(cls, config, cmd, env):
        """run a remote command and return a control object"""
        username = config.get('REMOTE_USER', None)
        userpass = config.get('REMOTE_PASS', None)

        ssh_client = para.SSHClient()
        ssh_client.set_missing_host_key_policy(para.RejectPolicy())
        ssh_client.connect(hostname=config['server_ip'], username=username,
                           password=userpass)

        _, stdout, stderr = ssh_client.exec_command(cmd, environment=env)

        return cls(ssh_client, stdout, stderr)
