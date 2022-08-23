---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_conn_wait.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.0.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_conn\_wait** - wait for a completion event on the shared
completion channel from CQ or RCQ

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_conn;
          struct rpma_cq
          int rpma_conn_wait(struct rpma_conn *conn, int flags, struct rpma_cq **cq, bool *is_rcq)

DESCRIPTION
===========

**rpma\_conn\_wait**() waits for a completion event on the shared
completion channel from CQ or RCQ, acks it and returns a CQ that caused
the event in the cq argument and a boolean value saying if it is RCQ or
not in the is\_rcq argument (if is\_rcq is not NULL). If
**rpma\_conn\_wait**() succeeds, then all available completions should
be collected from the returned cq using **rpma\_cq\_get\_wc**(3).

RETURN VALUE
============

The **rpma\_conn\_wait**() function returns 0 on success or a negative
error code on failure.

ERRORS
======

**rpma\_conn\_wait**() can fail with the following errors:

-   RPMA\_E\_INVAL - conn or cq are NULL

-   RPMA\_E\_NOT\_SHARED\_CHNL - the completion event channel is not
    shared

-   RPMA\_E\_NO\_COMPLETION - **ibv\_get\_cq\_event**(3) failed

-   RPMA\_E\_UNKNOWN - **ibv\_get\_cq\_event**(3) returned unknown CQ

-   RPMA\_E\_PROVIDER - **ibv\_req\_notify\_cq**(3) failed

SEE ALSO
========

**rpma\_conn\_req\_new**(3), **librpma**(7) and https://pmem.io/rpma/
