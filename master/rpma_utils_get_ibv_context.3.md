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

**rpma\_utils\_get\_ibv\_context** - obtain an RDMA device context by IP
address

SYNOPSIS
========

          #include <librpma.h>

          int rpma_utils_get_ibv_context(const char *addr,
                  enum rpma_util_ibv_context_type type, struct ibv_context **dev);

DESCRIPTION
===========

**rpma\_utils\_get\_ibv\_context**() obtains an RDMA device context by
the given IPv4/IPv6 address (either local or remote) using the TCP RDMA
port space (RDMA\_PS\_TCP) - reliable, connection-oriented and message
based QP communication.

RETURN VALUE
============

The **rpma\_utils\_get\_ibv\_context**() function returns 0 on success
or a negative error code on failure.
**rpma\_utils\_get\_ibv\_context**() does not set \**dev* value on
failure.

ERRORS
======

**rpma\_utils\_get\_ibv\_context**() can fail with the following errors:

-   RPMA\_E\_INVAL - *addr* or *dev* is NULL or *type* is unknown

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - **rdma\_getaddrinfo**(), **rdma\_create\_id**(),
    **rdma\_bind\_addr**() or **rdma\_resolve\_addr**() failed, errno
    can be checked using **rpma\_err\_get\_provider\_error**()
