---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_get_rcq.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_srq_get_rcq \-- get the receive CQ from the shared RQ object**

# SYNOPSIS

          #include <librpma.h>

          struct rpma_srq;
          struct rpma_cq;
          int rpma_srq_get_rcq(const struct rpma_srq *srq, struct rpma_cq **rcq_ptr);

# DESCRIPTION

**rpma_srq_get_rcq**() gets the receive CQ from the shared RQ object.
The receive CQ created by **rpma_srq_new**(3) allows handling all
**rpma_srq_recv**(3) completions within the shared RQ.
**rpma_srq_cfg_set_rcq_size**(3) can change the receive CQ size.

# RETURN VALUE

The **rpma_srq_get_rcq**() function returns 0 on success or a negative
error code on failure. **rpma_srq_get_rcq**() does not set \*rcq_ptr
value on failure.

# ERRORS

**rpma_srq_get_rcq**() can fail with the following error:

-   RPMA_E\_INVAL - srq or rcq_ptr is NULL

# SEE ALSO

**rpma_cq_wait**(3), **rpma_cq_get_wc**(3), **rpma_cq_get_fd**(3),
**rpma_srq_cfg_set_rcq_size**(3), **rpma_srq_new**(3), **librpma**(7)
and https://pmem.io/rpma/
