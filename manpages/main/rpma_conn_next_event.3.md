---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_next_event.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

# NAME

**rpma_conn_next_event** - obtain a connection status

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          enum rpma_conn_event {
                  RPMA_CONN_UNDEFINED = -1,
                  RPMA_CONN_ESTABLISHED,
                  RPMA_CONN_CLOSED,
                  RPMA_CONN_LOST,
                  RPMA_CONN_REJECTED,
                  RPMA_CONN_UNREACHABLE
          };

          int rpma_conn_next_event(struct rpma_conn *conn,
                          enum rpma_conn_event *event);

# DESCRIPTION

**rpma_conn_next_event**() obtains the next event from the connection.
Types of events:

-   RPMA_CONN_UNDEFINED - undefined connection event

-   RPMA_CONN_ESTABLISHED - connection established

-   RPMA_CONN_CLOSED - connection closed

-   RPMA_CONN_LOST - connection lost

-   RPMA_CONN_REJECTED - connection rejected

-   RPMA_CONN_UNREACHABLE - connection unreachable

# RETURN VALUE

The **rpma_conn_next_event**() function returns 0 on success or a
negative error code on failure.

# ERRORS

**rpma_conn_next_event**() can fail with the following errors:

-   RPMA_E\_INVAL - conn or event is NULL

-   RPMA_E\_UNKNOWN - unexpected event

-   RPMA_E\_PROVIDER - **rdma_get_cm_event**() or
    **rdma_ack_cm_event**() failed

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_conn_req_connect**(3), **rpma_conn_disconnect**(3),
**librpma**(7) and https://pmem.io/rpma/
