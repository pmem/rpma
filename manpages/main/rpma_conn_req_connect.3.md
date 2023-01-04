---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_req_connect.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_req_connect** - initiate processing the connection request

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn_req;
          struct rpma_conn_private_data;
          struct rpma_conn;
          int rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
                          const struct rpma_conn_private_data *pdata, struct rpma_conn **conn_ptr);

# DESCRIPTION

**rpma_conn_req_connect**() initiates processing the connection requests
both incoming and outgoing. The end of processing is signalled by the
RPMA_CONN_ESTABLISHED event via **rpma_conn_next_event**().

# RETURN VALUE

The **rpma_conn_req_connect**() function returns 0 on success or a
negative error code on failure. On success, the newly created connection
object is stored in \*conn_ptr whereas \*req_ptr is consumed and set to
NULL. On failure, **rpma_conn_req_connect**() does not set \*conn_ptr
whereas \*req_ptr is consumed and set to NULL.

# ERRORS

**rpma_conn_req_connect**() can fail with the following errors:

-   RPMA_E\_INVAL - req_ptr, \*req_ptr or conn_ptr is NULL

-   RPMA_E\_INVAL - pdata is not NULL whereas pdata-\>len == 0

-   RPMA_E\_NOMEM - out of memory

-   RPMA_E\_PROVIDER - initiating a connection request failed (active
    side only)

-   RPMA_E\_PROVIDER - accepting the connection request failed (passive
    side only)

-   RPMA_E\_PROVIDER - freeing a communication event failed (passive
    side only)

# SEE ALSO

**rpma_conn_apply_remote_peer_cfg**(3), **rpma_conn_delete**(3),
**rpma_conn_disconnect**(3), **rpma_conn_get_cq**(3),
**rpma_conn_get_event_fd**(3), **rpma_conn_get_private_data**(3),
**rpma_conn_get_rcq**(3), **rpma_conn_next_event**(3),
**rpma_conn_req_new**(3), **rpma_ep_next_conn_req**(3),
**rpma_flush**(3), **rpma_read**(3), **rpma_recv**(3), **rpma_send**(3),
**rpma_write**(3), **rpma_atomic_write**(3), **librpma**(7) and
https://pmem.io/rpma/
