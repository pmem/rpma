#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""common.py -- the runner's helpers (EXPERIMENTAL)"""

UNKNOWN_MODE_MSG = "An unexpected 'mode' value: {}"
NO_X_AXIS_MSG = \
    "SETTINGS_BY_MODE[{}] is missing a key defined as a list (x-axis)"

BS_VALUES = [256, 1024, 4096, 8192, 16384, 32768, 65536, 131072, 262144]
