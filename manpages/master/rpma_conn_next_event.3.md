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

**rpma\_conn\_next\_event** - obtain a connection status

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          enum rpma_conn_event {
                  RPMA_CONN_UNDEFINED = -1,
                  RPMA_CONN_ESTABLISHED,
                  RPMA_CONN_CLOSED,
                  RPMA_CONN_LOST,
                  RPMA_CONN_REJECTED,
          };

          int rpma_conn_next_event(struct rpma_conn *conn,
                          enum rpma_conn_event *event);

DESCRIPTION
===========

**rpma\_conn\_next\_event**() obtains the next event from the
connection. Types of events:

-   RPMA\_CONN\_UNDEFINED - undefined connection event

-   RPMA\_CONN\_ESTABLISHED - connection established

-   RPMA\_CONN\_CLOSED - connection closed

-   RPMA\_CONN\_LOST - connection lost

-   RPMA\_CONN\_REJECTED - connection rejected

RETURN VALUE
============

The **rpma\_conn\_next\_event**() function returns 0 on success or a
negative error code on failure.

ERRORS
======

**rpma\_conn\_next\_event**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or event is NULL

-   RPMA\_E\_UNKNOWN - unexpected event

-   RPMA\_E\_PROVIDER - **rdma\_get\_cm\_event**() or
    **rdma\_ack\_cm\_event**() failed

-   RPMA\_E\_NOMEM - out of memory

SEE ALSO
========

**rpma\_conn\_req\_connect**(3), **rpma\_conn\_disconnect**(3),
**librpma**(7) and https://pmem.io/rpma/
