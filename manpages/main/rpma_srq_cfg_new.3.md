---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_cfg_new.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_srq_cfg_new** - create a new shared RQ configuration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_srq_cfg;
          int rpma_srq_cfg_new(struct rpma_srq_cfg **cfg_ptr);

# DESCRIPTION

**rpma_srq_cfg_new**() creates a new shared RQ configuration object and
fills it with the default values:

            .rcq_size = 100
            .rq_size = 100

Note that **rpma_srq_new**(3) with the default rcq_size creates its own
receive CQ.

# RETURN VALUE

The **rpma_srq_cfg_new**() function returns 0 on success or a negative
error code on failure. **rpma_srq_cfg_new**() does not set \*cfg_ptr
value on failure.

# ERRORS

**rpma_srq_cfg_new**() can fail with the following error:

-   RPMA_E\_INVAL - cfg_ptr is NULL

-   RPMA_E\_NOMEM - out of memory

# SEE ALSO

**rpma_srq_cfg_delete**(3), **rpma_srq_cfg_get_rcq_size**(3),
**rpma_srq_cfg_get_rq_size**(3), **rpma_srq_cfg_set_rcq_size**(3),
**rpma_srq_cfg_set_rq_size**(3), **rpma_srq_new**(3), **librpma**(7) and
https://pmem.io/rpma/
