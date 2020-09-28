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

**rpma\_conn\_req\_recv** - initiate the receive operation

SYNOPSIS
========

          #include <librpma.h>

          const struct rpma_conn_req;
          const struct rpma_mr_local;
          int rpma_conn_req_recv(const struct rpma_conn_req *req,
                          const struct rpma_mr_local *dst, size_t offset,
                          size_t len, const void *op_context);

DESCRIPTION
===========

**rpma\_conn\_req\_recv**() initiates the receive operation. It prepares
a buffer for a message send from other side of the connection. Please
see **rpma\_send**(3). This is a variant of **rpma\_recv**(3) which may
be used before the connection is established.

RETURN VALUE
============

The **rpma\_conn\_req\_recv**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_conn\_req\_recv**() can fail with the following errors:

-   RPMA\_E\_INVAL - req or src or op\_context is NULL

-   RPMA\_E\_PROVIDER - **ibv\_post\_recv**(3) failed
