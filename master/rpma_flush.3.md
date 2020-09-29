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

**rpma\_flush** - initiate the flush operation

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_mr_remote;
          enum rpma_flush_type {
                  RPMA_FLUSH_TYPE_PERSISTENT,
                  RPMA_FLUSH_TYPE_VISIBILITY,
          };

          int rpma_flush(struct rpma_conn *conn,
                          struct rpma_mr_remote *dst, size_t dst_offset,
                          size_t len, enum rpma_flush_type type, int flags,
                          const void *op_context);

DESCRIPTION
===========

**rpma\_flush**() initiates the flush operation (finalizing a transfer
of data to the remote memory). Possible types of **rpma\_flush**()
operation:

-   RPMA\_FLUSH\_TYPE\_PERSISTENT - flush data down to the persistent
    domain

-   RPMA\_FLUSH\_TYPE\_VISIBILITY - flush data deep enough to make it
    visible on the remote node

RETURN VALUE
============

The **rpma\_flush**() function returns 0 on success or a negative error
code on failure.

ERRORS
======

**rpma\_flush**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or dst is NULL

-   RPMA\_E\_INVAL - unknown type value

-   RPMA\_E\_INVAL - flags are not set

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed

-   RPMA\_E\_NOSUPP - type is RPMA\_FLUSH\_TYPE\_PERSISTENT and the
    direct write to pmem is not supported
