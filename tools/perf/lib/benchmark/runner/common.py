#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""common.py -- the runner's helpers (EXPERIMENTAL)"""

class RemoteCmd:
    """run a command on a remote side over SSH"""

    @classmethod
    def run_sync(cls, config, cmd):
        """XXX"""
        # XXX run a remote command and wait till its complete

    @classmethod
    def run_async(cls, config, cmd):
        """XXX"""
        # XXX run a remote command and return a control object
        # XXX maybe it can be an instance of this class?
