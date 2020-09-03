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

**rpma\_conn\_cfg\_set\_timeout** - set connection establishment timeout

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg,
                  int timeout_ms);

DESCRIPTION
===========

ERRORS
======

**rpma\_conn\_cfg\_set\_timeout**() can fail with the following error:

-   RPMA\_E\_INVAL - *cfg* is NULL or *timeout\_ms* \< 0
