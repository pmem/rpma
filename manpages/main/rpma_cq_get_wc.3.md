---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_cq_get_wc.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_cq_get_wc** - receive one or more completions

# SYNOPSIS

          #include <librpma.h>

          struct rpma_cq;
          struct ibv_wc;

          int rpma_cq_get_wc(struct rpma_cq *cq, int num_entries, struct ibv_wc *wc,
                          int *num_entries_got);

# DESCRIPTION

**rpma_cq_get_wc**() polls the CQ for completions and returns the first
num_entries (or all available completions if the CQ contains fewer than
this number) in the wc array exactly like **ibv_poll_cq**(3) does. The
argument wc is a pointer to an array of ibv_wc structs, as defined in
\<infiniband/verbs.h>. The number of got completions is returned in the
num_entries_got argument if it is not NULL. It can be NULL only if
num_entries equals 1. All operations generate completions on error. The
operations posted with the RPMA_F\_COMPLETION_ALWAYS flag also generate
completions on success.

Note that if the provided cq is the main CQ and the receive CQ is
present on the same connection this function won\'t return IBV_WC_RECV
and IBV_WC_RECV_RDMA_WITH_IMM at any time. The receive CQ has to be used
instead to collect these completions. Please see the
**rpma_conn_get_rcq**(3) for details about the receive CQ.

# RETURN VALUE

The **rpma_cq_get_wc**() function returns 0 on success or a negative
error code on failure. On success, it saves all got completions and
their number into the wc and num_entries_got respectively. If the status
of a completion is not equal to IBV_WC_SUCCESS then only the following
attributes are valid: wr_id, status, qp_num, and vendor_err.

# ERRORS

**rpma_cq_get_wc**() can fail with the following errors:

-   RPMA_E\_INVAL - num_entries \< 1, cq or wc is NULL, num_entries > 1
    and num_entries_got is NULL

-   RPMA_E\_NO_COMPLETION - no completions available

-   RPMA_E\_PROVIDER - **ibv_poll_cq**(3) failed with a provider error

-   RPMA_E\_UNKNOWN - **ibv_poll_cq**(3) failed but no provider error is
    available

# SEE ALSO

**rpma_conn_get_cq**(3), **rpma_conn_get_rcq**(3),
**rpma_conn_req_recv**(3), **rpma_cq_wait**(3), **rpma_cq_get_fd**(3),
**rpma_flush**(3), **rpma_read**(3), **rpma_recv**(3), **rpma_send**(3),
**rpma_send_with_imm**(3), **rpma_write**(3), **rpma_atomic_write**(3),
**rpma_write_with_imm**(3), **librpma**(7) and https://pmem.io/rpma/
