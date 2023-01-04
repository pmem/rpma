---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_disconnect.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

# NAME

**rpma_conn_disconnect** - tear the connection down

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          int rpma_conn_disconnect(struct rpma_conn *conn);

# DESCRIPTION

**rpma_conn_disconnect**() tears the connection down.

-   It may initiate disconnecting the connection. In this case, the end
    of disconnecting is signalled by the RPMA_CONN_CLOSED event via
    **rpma_conn_next_event**() or

-   It may be called after receiving the RPMA_CONN_CLOSED event. In this
    case, the disconnection is done when **rpma_conn_disconnect**()
    returns with success.

# RETURN VALUE

The **rpma_conn_disconnect**() function returns 0 on success or a
negative error code on failure.

# ERRORS

**rpma_conn_disconnect**() can fail with the following errors:

-   RPMA_E\_INVAL - conn is NULL

-   RPMA_E\_PROVIDER - **rdma_disconnect**() failed

# SEE ALSO

**rpma_conn_delete**(3), **rpma_conn_next_event**(3),
**rpma_conn_req_connect**(3), **librpma**(7) and https://pmem.io/rpma/
