#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# common.py
#

"""the runner's helpers (EXPERIMENTAL)"""

#: an error message when an unexpected value of a key is detected
UNKNOWN_VALUE_MSG = "An unexpected '{}' value: {}"

#: an error message when x-axis cannot be identified
NO_X_AXIS_MSG = \
    "SETTINGS_BY_MODE[{}] is missing a key defined as a list (x-axis)"

#: an error message when a key is missing in the figure
MISSING_KEY_MSG = "the following key is missing in the figure: {}"

#: a common block sizes list
BS_VALUES = [256, 1024, 4096, 8192, 16384, 32768, 65536, 131072, 262144]
