---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_ep_next_conn_req.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.2.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_ep_next_conn_req** - obtain an incoming connection request

# SYNOPSIS

          #include <librpma.h>

          struct rpma_ep;
          struct rpma_conn_cfg;
          struct rpma_conn_req;
          int rpma_ep_next_conn_req(struct rpma_ep *ep, const struct rpma_conn_cfg *cfg,
                          struct rpma_conn_req **req_ptr);

# DESCRIPTION

**rpma_ep_next_conn_req**() obtains the next connection request from the
endpoint.

# RETURN VALUE

The **rpma_ep_next_conn_req**() function returns 0 on success or a
negative error code on failure. **rpma_ep_next_conn_req**() does not set
\*req_ptr value on failure.

# ERRORS

**rpma_ep_next_conn_req**() can fail with the following errors:

-   RPMA_E\_INVAL - ep or req_ptr is NULL

-   RPMA_E\_INVAL - obtained an event different than a connection
    request

-   RPMA_E\_PROVIDER - **rdma_get_cm_event**(3) failed

-   RPMA_E\_NOMEM - out of memory

-   RPMA_E\_NO_EVENT - no next connection request available

# SEE ALSO

**rpma_conn_cfg_new**(3), **rpma_conn_req_delete**(3),
**rpma_conn_req_connect**(3), **rpma_ep_listen**(3), **librpma**(7) and
https://pmem.io/rpma/
