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

**rpma\_read** - initiate the read operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_local;
          struct rpma_mr_remote;
          int rpma_read(struct rpma_conn *conn,
                          struct rpma_mr_local *dst, size_t dst_offset,
                          const struct rpma_mr_remote *src,  size_t src_offset,
                          size_t len, int flags, const void *op_context);

DESCRIPTION
===========

**rpma\_read**() initiates transferring data from the remote memory to
the local memory. The attribute flags set the completion notification
indicator:

-   RPMA\_F\_COMPLETION\_ON\_ERROR - generate the completion on error

-   RPMA\_F\_COMPLETION\_ALWAYS - generate the completion regardless of
    result of the operation

RETURN VALUE
============

The **rpma\_read**() function returns 0 on success or a negative error
code on failure.

ERRORS
======

**rpma\_read**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn, dst or src is NULL

-   RPMA\_E\_INVAL - flags are not set

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed

SEE ALSO
========

**rpma\_conn\_req\_connect**(3), **rpma\_mr\_reg**(3),
**rpma\_mr\_remote\_from\_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
