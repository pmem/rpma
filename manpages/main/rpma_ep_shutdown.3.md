---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_ep_shutdown.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_ep_shutdown** - stop listening and delete the endpoint

# SYNOPSIS

          #include <librpma.h>

          struct rpma_ep;
          int rpma_ep_shutdown(struct rpma_ep **ep_ptr);

# DESCRIPTION

**rpma_ep_shutdown**() stops listening for incoming connections and
deletes the endpoint. The connections established using the endpoint
will still exist after deleting the endpoint.

# RETURN VALUE

The **rpma_ep_shutdown**() function returns 0 on success or a negative
error code on failure. **rpma_ep_shutdown**() does not set \*ep_ptr
value to NULL on failure.

# ERRORS

**rpma_ep_shutdown**() can fail with the following errors:

-   RPMA_E\_INVAL - ep_ptr is NULL

-   RPMA_E\_PROVIDER - **rdma_destroy_id**(3) failed

# SEE ALSO

**rpma_ep_listen**(3), **librpma**(7) and https://pmem.io/rpma/
