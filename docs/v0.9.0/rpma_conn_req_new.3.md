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

          struct rpma_peer;
          struct rpma_conn_cfg;
          struct rpma_conn_req;
          int rpma_conn_req_new(struct rpma_peer *peer, const char *addr,
                          const char *port, const struct rpma_conn_cfg *cfg,
                          struct rpma_conn_req **req_ptr);

DESCRIPTION
===========

**rpma\_conn\_req\_new**() creates a new outgoing connection request
object using reliable, connection-oriented and message-based
(RDMA\_PS\_TCP) QP communication.

RETURN VALUE
============

The **rpma\_conn\_req\_new**() function returns 0 on success or a
negative error code on failure. **rpma\_conn\_req\_new**() does not set
\*req\_ptr value on failure.

ERRORS
======

**rpma\_conn\_req\_new**() can fail with the following errors:

-   RPMA\_E\_INVAL - peer, addr, port or req\_ptr is NULL

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - **rdma\_create\_id**(3),
    **rdma\_resolve\_addr**(3), **rdma\_resolve\_route**(3) or
    **ibv\_create\_cq**(3) failed

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_req\_connect**(3),
**rpma\_conn\_req\_delete**(3), **rpma\_conn\_req\_recv**(3),
**rpma\_ep\_next\_conn\_req**(3), **rpma\_peer\_new**(3), **librpma**(7)
and https://pmem.io/rpma/
