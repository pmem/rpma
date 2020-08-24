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

**rpma\_ep\_listen** - create a listening endpoint

SYNOPSIS
========

          #include <librpma.h>

          int rpma_ep_listen(const char *addr, const char *port,
              struct rpma_ep **ep);

DESCRIPTION
===========

Create an endpoint and initialize listening for incoming connections.

ERRORS
======

**rpma\_ep\_listen**() can fail with the following errors:

-   RPMA\_E\_INVAL - peer, *addr*, *port* or *ep* is NULL

-   RPMA\_E\_PROVIDER - **rdma\_create\_event\_channel**(3),
    **rdma\_create\_id**(3), **rdma\_getaddrinfo**(3),
    **rdma\_listen**(3) failed

-   RPMA\_E\_NOMEM - out of memory
