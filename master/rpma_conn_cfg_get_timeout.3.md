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

**rpma\_conn\_cfg\_get\_timeout** - get connection establishment timeout

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_conn_cfg;
          int rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg,
                          int *timeout_ms);

DESCRIPTION
===========

**rpma\_conn\_cfg\_get\_timeout**() gets the connection establishment
timeout.

RETURN VALUE
============

The **rpma\_conn\_cfg\_get\_timeout**() function returns 0 on success or
a negative error code on failure. **rpma\_conn\_cfg\_get\_timeout**()
does not set \*timeout\_ms value on failure.

ERRORS
======

**rpma\_conn\_cfg\_get\_timeout**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg or timeout\_ms is NULL
