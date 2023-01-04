---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_srq_cfg_set_rq_size.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_srq_cfg_set_rq_size** - set RQ size of the shared RQ

# SYNOPSIS

          #include <librpma.h>

          struct rpma_srq_cfg;
          int rpma_srq_cfg_set_rq_size(struct rpma_srq_cfg *cfg, uint32_t rq_size);

# DESCRIPTION

**rpma_srq_cfg_set_rq_size**() sets the RQ size of the shared RQ. If
this function is not called, the rq_size has the default value (100) set
by **rpma_srq_cfg_new**(3).

# RETURN VALUE

The **rpma_srq_cfg_set_rq_size**() function returns 0 on success or a
negative error code on failure.

# ERRORS

**rpma_srq_cfg_set_rq_size**() can fail with the following error:

-   RPMA_E\_INVAL - cfg is NULL

# SEE ALSO

**rpma_srq_cfg_new**(3), **rpma_srq_cfg_get_rq_size**(3), **librpma**(7)
and https://pmem.io/rpma/
