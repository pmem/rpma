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

**rpma\_recv** - initialize the receive operation

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_conn;
          const struct rpma_mr_local;
          int rpma_recv(const struct rpma_conn *conn,
                          const struct rpma_mr_local *dst, size_t offset,
                          size_t len, const void *op_context);

DESCRIPTION
===========

**rpma\_recv**() initializes the receive operation which prepares a
buffer for a message send from other side of the connection. Please see
**rpma\_send**(3).

All buffers prepared via **rpma\_recv**(3) form an unordered set. When a
message arrives it is placed in one of the buffers awaitaning and a
completion for the receive operation is generated.

A buffer for an incoming message have to be prepared beforehand.

The order of buffers in the set does not affect the order of completions
of receive operations get via **rpma\_conn\_next\_completion**(3).

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

-   RPMA\_E\_INVAL - conn or src is NULL

-   RPMA\_E\_PROVIDER - **ibv\_post\_recv**(3) failed
