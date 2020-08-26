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

          int rpma_send(struct rpma_conn *conn,
                  struct rpma_mr_local *src, size_t offset, size_t len,
                  int flags, void *op_context);

DESCRIPTION
===========

Initialize the send operation which transfers a message from the local
memory to other side of the connection.

ERRORS
======

**rpma\_send**() can fail with the following errors:

-   RPMA\_E\_INVAL - *conn* or *src* is NULL

-   RPMA\_E\_INVAL - *flags* are not set

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed
