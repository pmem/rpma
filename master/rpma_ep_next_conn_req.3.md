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

**rpma\_ep\_next\_conn\_req** - obtain an incoming connection request

SYNOPSIS
========

          #include <librpma.h>

          int rpma_ep_next_conn_req(struct rpma_ep *ep,
              struct rpma_conn_req **req);

DESCRIPTION
===========

Obtains the next connection request from the endpoint.

ERRORS
======

**rpma\_ep\_next\_conn\_req**() can fail with the following errors:

-   RPMA\_E\_INVAL - *ep* or *req* is NULL

-   RPMA\_E\_INVAL - obtained an event different than a connection
    request

-   RPMA\_E\_PROVIDER - **rdma\_get\_cm\_event**(3) failed

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_NO\_NEXT - no next connection request available
