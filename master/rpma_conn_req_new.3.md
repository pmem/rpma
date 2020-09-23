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

**rpma\_conn\_req\_new** - create a new outgoing connection request
object

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_req_new(const struct rpma_peer *peer, const char *addr,
                  const char *port, struct rpma_conn_req **req_ptr);

DESCRIPTION
===========

Create a new outgoing connection request object using reliable,
connection-oriented and message-based (RDMA\_PS\_TCP) QP communication.

ERRORS
======

**rpma\_conn\_req\_new**() can fail with the following errors:

-   RPMA\_E\_INVAL - *peer*, *addr*, *port* or *req\_ptr* is NULL

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - **rdma\_create\_id**(3),
    **rdma\_resolve\_addr**(3), **rdma\_resolve\_route**(3) or
    **ibv\_create\_cq**(3) failed
