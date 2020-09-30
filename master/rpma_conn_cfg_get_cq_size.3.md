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

**rpma\_conn\_cfg\_get\_cq\_size** - get CQ size for the connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_get_cq_size(const struct rpma_conn_cfg *cfg,
                          uint32_t *cq_size);

DESCRIPTION
===========

**rpma\_conn\_cfg\_get\_cq\_size**() gets the CQ size for the
connection.

RETURN VALUE
============

The **rpma\_conn\_cfg\_get\_cq\_size**() function returns 0 on success
or a negative error code on failure.
**rpma\_conn\_cfg\_get\_cq\_size**() does not set \*cq\_size value on
failure.

ERRORS
======

**rpma\_conn\_cfg\_get\_cq\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg or cq\_size is NULL

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_cfg\_set\_cq\_size**(3) and
https://pmem.io/rpma/
