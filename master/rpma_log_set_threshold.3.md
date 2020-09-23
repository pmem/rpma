---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_log\_set\_threshold** - set the logging threshold level

SYNOPSIS
========

          #include <librpma.h>

          typedef enum {
                  RPMA_LOG_DISABLED,
                  RPMA_LOG_LEVEL_FATAL,
                  RPMA_LOG_LEVEL_ERROR,
                  RPMA_LOG_LEVEL_WARNING,
                  RPMA_LOG_LEVEL_NOTICE,
                  RPMA_LOG_LEVEL_INFO,
                  RPMA_LOG_LEVEL_DEBUG,
          } rpma_log_level;

          typedef enum {
                  RPMA_LOG_THRESHOLD,
                  RPMA_LOG_THRESHOLD_AUX,
                  RPMA_LOG_THRESHOLD_MAX
