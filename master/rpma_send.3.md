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

**rpma\_send** - initialize the send operation

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_conn;
          const struct rpma_mr_local;
          int rpma_send(const struct rpma_conn *conn,
                          const struct rpma_mr_local *src, size_t offset,
                          size_t len, int flags, const void *op_context);

DESCRIPTION
===========

**rpma\_send**() initializes the send operation which transfers a
message from the local memory to other side of the connection.

RETURN VALUE
============

The **rpma\_send**() function returns 0 on success or a negative error
code on failure.

ERRORS
======

**rpma\_send**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or src is NULL

-   RPMA\_E\_INVAL - flags are not set

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed
