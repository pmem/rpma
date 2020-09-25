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

**rpma\_conn\_cfg\_get\_rq\_size** - get RQ size for the connection

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_conn_cfg;
          int rpma_conn_cfg_get_rq_size(const struct rpma_conn_cfg *cfg,
                          uint32_t *rq_size);

DESCRIPTION
===========

**rpma\_conn\_cfg\_get\_rq\_size**() gets the RQ size for the
connection.

RETURN VALUE
============

The **rpma\_conn\_cfg\_get\_rq\_size**() function returns 0 on success
or a negative error code on failure.
**rpma\_conn\_cfg\_get\_rq\_size**() does not set \*rq\_size value on
failure.

ERRORS
======

**rpma\_conn\_cfg\_get\_rq\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg or rq\_size is NULL
