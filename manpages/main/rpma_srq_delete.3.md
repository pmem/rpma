---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_delete.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_srq_delete** - delete the shared RQ object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_srq;
          int rpma_conn_req_delete(struct rpma_srq **srq_ptr);

# DESCRIPTION

**rpma_srq_delete**() deletes the shared RQ object.

# RETURN VALUE

The **rpma_srq_delete**() function returns 0 on success or a negative
error code on failure. **rpma_srq_delete**() sets \*srq_ptr value to
NULL on success and on failure.

# ERRORS

**rpma_srq_delete**() can fail with the following errors:

-   RPMA_E\_INVAL - srq_ptr is NULL

-   RPMA_E\_PROVIDER - **ibv_destroy_cq**(3),
    **ibv_destroy_comp_channel**(3), or **ibv_destroy_srq**(3) failed

# SEE ALSO

**rpma_srq_new**(3), **rpma_srq_get_rcq**(3), **librpma**(7) and
https://pmem.io/rpma/
