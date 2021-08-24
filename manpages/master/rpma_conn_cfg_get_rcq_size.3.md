---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.9.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_conn\_cfg\_get\_rcq\_size** - get receive CQ size for the
connection

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_cfg;
          int rpma_conn_cfg_get_rcq_size(const struct rpma_conn_cfg *cfg,
                          uint32_t *rcq_size);

DESCRIPTION
===========

**rpma\_conn\_cfg\_get\_rcq\_size**() gets the receive CQ size for the
connection. Please see the **rpma\_conn\_get\_rcq**() for details about
the receive CQ.

RETURN VALUE
============

The **rpma\_conn\_cfg\_get\_rcq\_size**() function returns 0 on success
or a negative error code on failure.
**rpma\_conn\_cfg\_get\_rcq\_size**() does not set \*rcq\_size value on
failure.

ERRORS
======

**rpma\_conn\_cfg\_get\_rcq\_size**() can fail with the following error:

-   RPMA\_E\_INVAL - cfg or rcq\_size is NULL

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_cfg\_set\_rcq\_size**(3),
**rpma\_conn\_get\_rcq**(3), **librpma**(7) and https://pmem.io/rpma/
