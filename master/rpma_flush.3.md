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

**rpma\_flush** - initialize the flush operation

SYNOPSIS
========

          #include <librpma.h>

          int rpma_flush(const struct rpma_conn *conn,
                  const struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
                  enum rpma_flush_type type, int flags, const void *op_context);

DESCRIPTION
===========

Initialize the flush operation (finalizing a transfer of data to the
remote memory).

ERRORS
======

**rpma\_flush**() can fail with the following errors:

-   RPMA\_E\_INVAL - *conn* or *dst* is NULL

-   RPMA\_E\_INVAL - unknown *type* value

-   RPMA\_E\_INVAL - *flags* are not set

-   RPMA\_E\_PROVIDER - **ibv\_post\_send**(3) failed

-   RPMA\_E\_NOSUPP - *type* is RPMA\_FLUSH\_TYPE\_PERSISTENT and the
    direct write to pmem is not supported
