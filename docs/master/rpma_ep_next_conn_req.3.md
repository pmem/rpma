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

          struct rpma_ep;
          struct rpma_conn_cfg;
          struct rpma_conn_req;
          int rpma_ep_next_conn_req(struct rpma_ep *ep,
                          const struct rpma_conn_cfg *cfg,
                          struct rpma_conn_req **req_ptr);

DESCRIPTION
===========

**rpma\_ep\_next\_conn\_req**() obtains the next connection request from
the endpoint.

RETURN VALUE
============

The **rpma\_ep\_next\_conn\_req**() function returns 0 on success or a
negative error code on failure. **rpma\_ep\_next\_conn\_req**() does not
set \*req\_ptr value on failure.

ERRORS
======

**rpma\_ep\_next\_conn\_req**() can fail with the following errors:

-   RPMA\_E\_INVAL - ep or req\_ptr is NULL

-   RPMA\_E\_INVAL - obtained an event different than a connection
    request

-   RPMA\_E\_PROVIDER - **rdma\_get\_cm\_event**(3) failed

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_NO\_EVENT - no next connection request available

SEE ALSO
========

**rpma\_conn\_cfg\_new**(3), **rpma\_conn\_req\_delete**(3),
**rpma\_conn\_req\_connect**(3), **rpma\_ep\_listen**(3), **librpma**(7)
and https://pmem.io/rpma/
