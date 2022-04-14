---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_cq_wait.3.html"]
title: "librpma | PMDK"
header: "librpma API version 0.14.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

NAME
====

**rpma\_cq\_wait** - wait for a completion

SYNOPSIS
========

          #include <librpma.h>

          struct rpma_cq;
          int rpma_cq_wait(struct rpma_cq *cq);

DESCRIPTION
===========

**rpma\_cq\_wait**() waits for an incoming completion. If it succeeds,
then all available completions should be collected using
**rpma\_cq\_get\_wc**(3) before the next **rpma\_cq\_wait**() call.

RETURN VALUE
============

The **rpma\_cq\_wait**() function returns 0 on success or a negative
error code on failure.

ERRORS
======

**rpma\_cq\_wait**() can fail with the following errors:

-   RPMA\_E\_INVAL - cq is NULL

-   RPMA\_E\_PROVIDER - **ibv\_req\_notify\_cq**(3) failed with a
    provider error

-   RPMA\_E\_NO\_COMPLETION - no completions available

-   RPMA\_E\_SHARED\_CHANNEL - cannot proceed, because the completion
    channel is shared

SEE ALSO
========

**rpma\_conn\_get\_cq**(3), **rpma\_conn\_get\_rcq**(3),
**rpma\_cq\_get\_wc**(3), **rpma\_cq\_get\_fd**(3), **librpma**(7) and
https://pmem.io/rpma/
