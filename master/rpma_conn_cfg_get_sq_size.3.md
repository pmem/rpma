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

**rpma\_conn\_cfg\_get\_sq\_size** - get SQ size for the connection

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_cfg_get_sq_size(struct rpma_conn_cfg *cfg,
                  uint32_t *sq_size);

DESCRIPTION
===========

ERRORS
======

**rpma\_conn\_cfg\_get\_sq\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - *cfg* or *sq\_size* is NULL
