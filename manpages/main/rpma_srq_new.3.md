---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_srq_new** - create a new shared RQ object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_peer;
          struct rpma_srq_cfg;
          struct rpma_srq;
          int rpma_srq_new(struct rpma_peer *peer, const struct rpma_srq_cfg *cfg,
                          struct rpma_srq **srq_ptr);

# DESCRIPTION

**rpma_srq_new**() creates a new shared RQ object including a new shared
RQ and a new shared receive CQ. It does not create the shared receive CQ
if the size of the receive CQ in cfg equals 0.

# RETURN VALUE

The **rpma_srq_new**() function returns 0 on success or a negative error
code on failure. **rpma_srq_new**() does not set \*srq_ptr value on
failure. If cfg is NULL, then the default values are used

-   see **rpma_srq_cfg_new**(3) for more details.

# ERRORS

**rpma_srq_new**() can fail with the following errors:

-   RPMA_E\_INVAL - peer or srq_ptr is NULL

-   RPMA_E\_NOMEM - out of memory

-   RPMA_E\_PROVIDER - **ibv_create_srq**(3),
    **ibv_create_comp_channel**(3), **ibv_create_cq**(3) or
    **ibv_req_notify_cq**(3) failed

# SEE ALSO

**rpma_srq_delete**(3), **rpma_srq_get_rcq**(3), **librpma**(7) and
https://pmem.io/rpma/
