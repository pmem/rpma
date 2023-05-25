---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_cq_wait.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_cq_wait** - wait for a completion and ack it

# SYNOPSIS

          #include <librpma.h>

          struct rpma_cq;
          int rpma_cq_wait(struct rpma_cq *cq);

# DESCRIPTION

**rpma_cq_wait**() waits for an incoming completion event and acks it.
If **rpma_cq_wait**() succeeds, then all available completions should be
collected using **rpma_cq_get_wc**(3) before the next **rpma_cq_wait**()
call.

# RETURN VALUE

The **rpma_cq_wait**() function returns 0 on success or a negative error
code on failure.

# ERRORS

**rpma_cq_wait**() can fail with the following errors:

-   RPMA_E\_INVAL - cq is NULL

-   RPMA_E\_PROVIDER - **ibv_req_notify_cq**(3) failed with a provider
    error

-   RPMA_E\_NO_COMPLETION - no completions available

-   RPMA_E\_SHARED_CHANNEL - the completion event channel is shared and
    cannot be handled by any particular CQ

# SEE ALSO

**rpma_conn_get_cq**(3), **rpma_conn_get_rcq**(3),
**rpma_cq_get_wc**(3), **rpma_cq_get_fd**(3), **librpma**(7) and
https://pmem.io/rpma/
