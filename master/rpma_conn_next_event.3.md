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

          int rpma_conn_next_event(struct rpma_conn *conn,
                  enum rpma_conn_event *event);

DESCRIPTION
===========

Obtain the next *event* from the connection

ERRORS
======

**rpma\_conn\_next\_event**() can fail with the following errors:

-   RPMA\_E\_INVAL - *conn* or *event* is NULL

-   RPMA\_E\_UNKNOWN - unexpected *event*

-   RPMA\_E\_PROVIDER - **rdma\_get\_cm\_event**() or
    **rdma\_ack\_cm\_event**() failed

-   RPMA\_E\_NOMEM - out of memory
