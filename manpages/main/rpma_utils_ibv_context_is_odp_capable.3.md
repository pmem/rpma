---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_utils_ibv_context_is_odp_capable.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_utils_ibv_context_is_odp_capable** - is On-Demand Paging
supported

# SYNOPSIS

          #include <librpma.h>

          struct ibv_context;
          int rpma_utils_ibv_context_is_odp_capable(struct ibv_context *ibv_ctx,
                  int *is_odp_capable);

# DESCRIPTION

**rpma_utils_ibv_context_is_odp_capable**() queries the RDMA device
context\'s capabilities and check if it supports On-Demand Paging.

# RETURN VALUE

The **rpma_utils_ibv_context_is_odp_capable**() function returns 0 on
success or a negative error code on failure. The \*is_odp_capable value
on failure is undefined.

# ERRORS

**rpma_utils_ibv_context_is_odp_capable**() can fail with the following
errors:

-   RPMA_E\_INVAL - ibv_ctx or is_odp_capable is NULL

-   RPMA_E\_PROVIDER - **ibv_query_device_ex**() failed, the exact cause
    of the error can be read from the log

# SEE ALSO

**rpma_utils_get_ibv_context**(3), **librpma**(7) and
https://pmem.io/rpma/
