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

**rpma\_conn\_req\_delete** - delete the connection request

SYNOPSIS
========

          #include <librpma.h>

          int rpma_conn_req_delete(struct rpma_conn_req **req_ptr);

DESCRIPTION
===========

Delete the connection request both incoming and outgoing.

ERRORS
======

**rpma\_conn\_req\_delete**() can fail with the following errors:

-   RPMA\_E\_INVAL - *req\_ptr* is NULL

-   RPMA\_E\_PROVIDER

    -   **rdma\_destroy\_qp**(3) or **ibv\_destroy\_cq**(3) failed

    -   **rdma\_reject**(3) or **rdma\_ack\_cm\_event**(3) failed
        (passive side only)

    -   **rdma\_destroy\_id**(3) failed (active side only)
