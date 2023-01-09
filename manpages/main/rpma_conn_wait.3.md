---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_wait.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_conn_wait** - wait for a completion event on the shared
completion channel from CQ or RCQ

# SYNOPSIS

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_cq
          int rpma_conn_wait(struct rpma_conn *conn, int flags, struct rpma_cq **cq, bool *is_rcq)

# DESCRIPTION

**rpma_conn_wait**() waits for a completion event on the shared
completion channel from CQ or RCQ, acks it and returns a CQ that caused
the event in the cq argument and a boolean value saying if it is RCQ or
not in the is_rcq argument (if is_rcq is not NULL). If
**rpma_conn_wait**() succeeds, then all available completions should be
collected from the returned cq using **rpma_cq_get_wc**(3).

# RETURN VALUE

The **rpma_conn_wait**() function returns 0 on success or a negative
error code on failure.

# ERRORS

**rpma_conn_wait**() can fail with the following errors:

-   RPMA_E\_INVAL - conn or cq are NULL

-   RPMA_E\_NOT_SHARED_CHNL - the completion event channel is not shared

-   RPMA_E\_NO_COMPLETION - **ibv_get_cq_event**(3) failed

-   RPMA_E\_UNKNOWN - **ibv_get_cq_event**(3) returned unknown CQ

-   RPMA_E\_PROVIDER - **ibv_req_notify_cq**(3) failed

# SEE ALSO

**rpma_conn_req_new**(3), **librpma**(7) and https://pmem.io/rpma/
