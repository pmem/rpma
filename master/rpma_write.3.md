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

**rpma\_write** - initiate the write operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_local;
          struct rpma_mr_remote;
          int rpma_write(struct rpma_conn *conn,
                          struct rpma_mr_remote *dst, size_t dst_offset,
                          const struct rpma_mr_local *src,  size_t src_offset,
                          size_t len, int flags, const void *op_context);

DESCRIPTION
===========

**rpma\_write**() initiates the write operation (transferring data from
the local memory to the remote memory).

RETURN VALUE
============

The **rpma\_write**() function returns 0 on success or a negative error
code on failure.

ERRORS
======

**rpma\_write**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn, dst or src is NULL

-   RPMA\_E\_INVAL - flags are not set

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed

SEE ALSO
========

**rpma\_conn\_req\_connect**(3), **rpma\_mr\_reg**(3),
**rpma\_mr\_remote\_from\_descriptor**(3) and https://pmem.io/rpma/
