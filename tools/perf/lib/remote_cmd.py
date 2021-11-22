#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""remote_cmd.py -- running commands on remote node over SSH (EXPERIMENTAL)"""

import paramiko as p

class RemoteCmd:
    """run a command on a remote side over SSH"""

    def __init__(self, ssh_client, stdout, stderr, exit_status):
        self.ssh_client = ssh_client
        self.stdout = stdout
        self.stderr = stderr
        self.exit_status = exit_status

    def wait(self):
        """wait for the command to finish"""
        self.exit_status = self.stdout.channel.recv_exit_status()
        self.ssh_client.close()
        self.ssh_client = None

    @classmethod
    def run_sync(cls, config, cmd):
        """run a remote command and return a control object"""
        remoteCmd = RemoteCmd.run_async(config, cmd)
        remoteCmd.wait()
        return remoteCmd

    @classmethod
    def run_async(cls, config, cmd):
        """run a remote command and return a control object"""
        if 'REMOTE_USER' in config:
            username = config['REMOTE_USER']
        else:
            username = None
        if 'REMOTE_PASS' in config:
            userpass = config['REMOTE_PASS']
        else:
            userpass = None
        if 'key_filename' in config:
            key_filename = config['key_filename']
        else:
            key_filename = None

        try:
            ssh_client = p.SSHClient()
            ssh_client.set_missing_host_key_policy(p.AutoAddPolicy())
            ssh_client.connect(hostname=config['server_ip'],
                           username=username, password=userpass,
                           key_filename=key_filename)
        except p.AuthenticationException:
            raise Exception("Authentication failed, please verify your credentials: %s" %username)
        except p.SSHException as sshException:
            raise Exception("Unable to establish SSH connection: %s" % sshException)
        except p.BadHostKeyException as badHostKeyException:
            raise Exception("Unable to verify server's host key: %s" % badHostKeyException)

        _,stdout,stderr=ssh_client.exec_command(cmd)

        return cls(ssh_client, stdout, stderr, None)
