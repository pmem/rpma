#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# common.py
#

"""the runner's helpers (EXPERIMENTAL)"""

#: an error message when an unexpected mode is detected
UNKNOWN_MODE_MSG = "An unexpected 'mode' value: {}"

#: an error message when x-axis cannot be identified
NO_X_AXIS_MSG = \
    "SETTINGS_BY_MODE[{}] is missing a key defined as a list (x-axis)"

#: a common block sizes list
BS_VALUES = [256, 1024, 4096, 8192, 16384, 32768, 65536, 131072, 262144]
CPU_LOAD_00_99 = [0, 25, 50, 75, 99]
THREADS_VALUES = [1, 2, 4, 8, 12, 16, 32, 64]
