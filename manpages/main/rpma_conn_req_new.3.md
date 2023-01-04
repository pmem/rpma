---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_req_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

# NAME

**rpma_conn_req_new **- create a new outgoing connection request object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_conn_cfg;
          struct rpma_conn_req;
          int rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *port,
                          const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr);

# DESCRIPTION

**rpma_conn_req_new**() creates a new outgoing connection request object
using reliable, connection-oriented and message-based (RDMA_PS_TCP) QP
communication.

# RETURN VALUE

The **rpma_conn_req_new**() function returns 0 on success or a negative
error code on failure. **rpma_conn_req_new**() does not set \*req_ptr
value on failure. If cfg is NULL, then the default values are used

-   see **rpma_conn_cfg_new**(3) for more details.

# ERRORS

**rpma_conn_req_new**() can fail with the following errors:

-   RPMA_E\_INVAL - peer, addr, port or req_ptr is NULL

-   RPMA_E\_NOMEM - out of memory

-   RPMA_E\_PROVIDER - **rdma_create_id**(3), **rdma_resolve_addr**(3),
    **rdma_resolve_route**(3) or **ibv_create_cq**(3) failed

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_req_connect**(3),
**rpma_conn_req_delete**(3), **rpma_conn_req_recv**(3),
**rpma_ep_next_conn_req**(3), **rpma_peer_new**(3), **librpma**(7) and
https://pmem.io/rpma/
