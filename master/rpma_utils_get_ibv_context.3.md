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

          struct ibv_context;
          enum rpma_util_ibv_context_type {
                  RPMA_UTIL_IBV_CONTEXT_LOCAL,
                  RPMA_UTIL_IBV_CONTEXT_REMOTE
          };

          int rpma_utils_get_ibv_context(const char *addr,
                  enum rpma_util_ibv_context_type type,
                  struct ibv_context **dev_ptr);

DESCRIPTION
===========

**rpma\_utils\_get\_ibv\_context**() obtains an RDMA device context by
the given IPv4/IPv6 address (either local or remote) using the TCP RDMA
port space (RDMA\_PS\_TCP) - reliable, connection-oriented and
message-based QP communication. Possible values of the \'type\'
argument:

-   RPMA\_UTIL\_IBV\_CONTEXT\_LOCAL - lookup for a device based on the
    given local address

-   RPMA\_UTIL\_IBV\_CONTEXT\_REMOTE - lookup for a device based on the
    given remote address

RETURN VALUE
============

The **rpma\_utils\_get\_ibv\_context**() function returns 0 on success
or a negative error code on failure.
**rpma\_utils\_get\_ibv\_context**() does not set \*dev\_ptr value on
failure.

ERRORS
======

**rpma\_utils\_get\_ibv\_context**() can fail with the following errors:

-   RPMA\_E\_INVAL - addr or dev\_ptr is NULL or type is unknown

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - **rdma\_getaddrinfo**(), **rdma\_create\_id**(),
    **rdma\_bind\_addr**() or **rdma\_resolve\_addr**() failed, the
    exact cause of the error can be read from the log
