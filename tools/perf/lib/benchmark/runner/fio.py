#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""fio.py -- the FIO runner (EXPERIMENTAL)"""

class FioRunner:
    """XXX"""

    def __init__(self, benchmark, config, idfile):
        """XXX"""
        raise NotImplementedError("The FioRunner is not in place yet")

    def run(self):
        """XXX"""
        # - set of args + vector of values for one of them
        # - op = read / write / read + write

        # for v in vector
        #     - set of args
        #     - prepare env
        #     - prepare args
        #     - server command
        #     - remote command pre
        #     - scp fio job to the server
        #     - server command + threading
        #     - client command
        #     - remote command post
        #     - JSON -> op -> JSON
