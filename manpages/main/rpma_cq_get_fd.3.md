---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_cq_get_fd.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_cq_get_fd** - get the completion queue\'s file descriptor

# SYNOPSIS

          #include <librpma.h>

          struct rpma_cq;
          int rpma_cq_get_fd(const struct rpma_cq *cq, int *fd);

# DESCRIPTION

**rpma_cq_get_fd**() gets the file descriptor of the completion queue
(CQ in short). When a next completion in the CQ is ready to be consumed
by **rpma_cq_get_wc**(3), the notification is delivered via the file
descriptor. The default mode of the file descriptor is blocking but it
can be changed to non-blocking mode using **fcntl**(2). The CQ is either
the connection\'s main CQ or the receive CQ or CQ of shared RQ, please
see **rpma_conn_get_cq**(3), **rpma_conn_get_rcq**(3) or
**rpma_srq_get_rcq**() for details.

Note after spotting the notification using the provided file descriptor
you do not have to call **rpma_cq_wait**(3) before consuming the
completion but it may cause that the next call to **rpma_cq_wait**(3)
will notify you of already consumed completion.

# RETURN VALUE

The **rpma_cq_get_fd**() function returns 0 on success or a negative
error code on failure. **rpma_cq_get_fd**() does not set \*fd value on
failure.

# ERRORS

**rpma_cq_get_fd**() can fail with the following error:

-   RPMA_E\_INVAL - cq or fd is NULL

# SEE ALSO

**fcntl**(2), **rpma_conn_get_cq**(3), **rpma_conn_get_rcq**(3),
**rpma_srq_get_rcq**(3), **rpma_cq_wait**(3), **rpma_cq_get_wc**(3),
**librpma**(7) and https://pmem.io/rpma/
