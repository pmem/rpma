---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_cq_get_wc.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_cq\_get\_wc** - receive one or more completions

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_cq;
          struct ibv_wc;

          int rpma_cq_get_wc(struct rpma_cq *cq, int num_entries, struct ibv_wc *wc,
                          int *num_entries_got);

DESCRIPTION
===========

**rpma\_cq\_get\_wc**() polls the CQ for completions and returns the
first num\_entries (or all available completions if the CQ contains
fewer than this number) in the wc array exactly like
**ibv\_poll\_cq**(3) does. The argument wc is a pointer to an array of
ibv\_wc structs, as defined in \<infiniband/verbs.h\>. The number of got
completions is returned in the num\_entries\_got argument if it is not
NULL. It can be NULL only if num\_entries equals 1. All operations
generate completions on error. The operations posted with the
RPMA\_F\_COMPLETION\_ALWAYS flag also generate completions on success.

Note that if the provided cq is the main CQ and the receive CQ is
present on the same connection this function won\'t return IBV\_WC\_RECV
and IBV\_WC\_RECV\_RDMA\_WITH\_IMM at any time. The receive CQ has to be
used instead to collect these completions. Please see the
**rpma\_conn\_get\_rcq**(3) for details about the receive CQ.

RETURN VALUE
============

The **rpma\_cq\_get\_wc**() function returns 0 on success or a negative
error code on failure. On success, it saves all got completions and
their number into the wc and num\_entries\_got respectively. If the
status of a completion is not equal to IBV\_WC\_SUCCESS then only the
following attributes are valid: wr\_id, status, qp\_num, and
vendor\_err.

ERRORS
======

**rpma\_cq\_get\_wc**() can fail with the following errors:

-   RPMA\_E\_INVAL - num\_entries \< 1, cq or wc is NULL,
    num\_entries \> 1 and num\_entries\_got is NULL

-   RPMA\_E\_NO\_COMPLETION - no completions available

-   RPMA\_E\_PROVIDER - **ibv\_poll\_cq**(3) failed with a provider
    error

-   RPMA\_E\_UNKNOWN - **ibv\_poll\_cq**(3) failed but no provider error
    is available

SEE ALSO
========

**rpma\_conn\_get\_cq**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_conn\_req\_recv**(3), **rpma\_cq\_wait**(3),
**rpma\_cq\_get\_fd**(3), **rpma\_flush**(3), **rpma\_read**(3),
**rpma\_recv**(3), **rpma\_send**(3), **rpma\_send\_with\_imm**(3),
**rpma\_write**(3), **rpma\_atomic\_write**(3),
**rpma\_write\_with\_imm**(3), **librpma**(7) and https://pmem.io/rpma/
