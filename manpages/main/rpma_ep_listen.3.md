---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_ep_listen.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_ep_listen** - create a listening endpoint

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_ep;
          int rpma_ep_listen(struct rpma_peer *peer, const char *addr, const char *port,
                          struct rpma_ep **ep_ptr);

# DESCRIPTION

**rpma_ep_listen**() creates an endpoint and initiates listening for
incoming connections using reliable, connection-oriented and
message-based (RDMA_PS_TCP) QP communication.

# RETURN VALUE

The **rpma_ep_listen**() function returns 0 on success or a negative
error code on failure. **rpma_ep_listen**() does not set \*ep_ptr value
on failure.

# ERRORS

**rpma_ep_listen**() can fail with the following errors:

-   RPMA_E\_INVAL - peer, addr, port or ep_ptr is NULL

-   RPMA_E\_PROVIDER - **rdma_create_event_channel**(3),
    **rdma_create_id**(3), **rdma_getaddrinfo**(3), **rdma_listen**(3)
    failed

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_ep_get_fd**(3), **rpma_ep_next_conn_req**(3),
**rpma_ep_shutdown**(3), **rpma_peer_new**(3), **librpma**(7) and
https://pmem.io/rpma/
