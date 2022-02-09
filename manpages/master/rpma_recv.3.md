---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: librpma API version 0.11.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_recv** - initiate the receive operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_local;
          int rpma_recv(struct rpma_conn *conn,
                          struct rpma_mr_local *dst, size_t offset,
                          size_t len, const void *op_context);

DESCRIPTION
===========

**rpma\_recv**() initiates the receive operation which prepares a buffer
for a message sent from other side of the connection. Please see
**rpma\_send**(3).

All buffers prepared via **rpma\_recv**(3) form an unordered set. When a
message arrives it is placed in one of the buffers awaitaning and a
completion for the receive operation is generated.

A buffer for an incoming message have to be prepared beforehand.

The order of buffers in the set does not affect the order of completions
of receive operations get via **rpma\_cq\_get\_wc**(3).

NOTE
====

In the RDMA standard, receive requests form an ordered queue. The RPMA
does NOT inherit this guarantee.

RETURN VALUE
============

The **rpma\_recv**() function returns 0 on success or a negative error
code on failure.

ERRORS
======

**rpma\_recv**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn == NULL

-   RPMA\_E\_INVAL - dst == NULL && (offset != 0 \|\| len != 0)

-   RPMA\_E\_PROVIDER - **ibv\_post\_recv**(3) failed

SEE ALSO
========

**rpma\_conn\_req\_connect**(3), **rpma\_mr\_reg**(3), **librpma**(7)
and https://pmem.io/rpma/
