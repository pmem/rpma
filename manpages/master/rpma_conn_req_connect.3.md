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

**rpma\_conn\_req\_connect** - initiate processing the connection
request

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn_req;
          struct rpma_conn_private_data;
          struct rpma_conn;
          int rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
                          const struct rpma_conn_private_data *pdata,
                          struct rpma_conn **conn_ptr);

DESCRIPTION
===========

**rpma\_conn\_req\_connect**() initiates processing the connection
requests both incoming and outgoing. The end of processing is signalled
by the RPMA\_CONN\_ESTABLISHED event via **rpma\_conn\_next\_event**().

RETURN VALUE
============

The **rpma\_conn\_req\_connect**() function returns 0 on success or a
negative error code on failure. On success, the newly created connection
object is stored in \*conn\_ptr whereas \*req\_ptr is consumed and set
to NULL. On failure, **rpma\_conn\_req\_connect**() does not set
\*conn\_ptr whereas \*req\_ptr is consumed and set to NULL.

ERRORS
======

**rpma\_conn\_req\_connect**() can fail with the following errors:

-   RPMA\_E\_INVAL - req\_ptr, \*req\_ptr or conn\_ptr is NULL

-   RPMA\_E\_INVAL - pdata is not NULL whereas pdata-\>len == 0

-   RPMA\_E\_NOMEM - out of memory

-   RPMA\_E\_PROVIDER - initiating a connection request failed (active
    side only)

-   RPMA\_E\_PROVIDER - accepting the connection request failed (passive
    side only)

-   RPMA\_E\_PROVIDER - freeing a communication event failed (passive
    side only)

SEE ALSO
========

**rpma\_conn\_apply\_remote\_peer\_cfg**(3), **rpma\_conn\_delete**(3),
**rpma\_conn\_disconnect**(3), **rpma\_conn\_get\_completion\_fd**(3),
**rpma\_conn\_get\_event\_fd**(3),
**rpma\_conn\_get\_private\_data**(3),
**rpma\_conn\_completion\_get**(3), **rpma\_conn\_next\_event**(3),
**rpma\_conn\_completion\_wait**(3), **rpma\_conn\_req\_new**(3),
**rpma\_ep\_next\_conn\_req**(3), **rpma\_flush**(3), **rpma\_read**(3),
**rpma\_recv**(3), **rpma\_send**(3), **rpma\_write**(3),
**rpma\_write\_atomic**(3), **librpma**(7) and https://pmem.io/rpma/
