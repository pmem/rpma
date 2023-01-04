---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_get_descriptor_size.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_get_descriptor_size** - get size of the memory region
descriptor

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr, size_t *desc_size);

# DESCRIPTION

**rpma_mr_get_descriptor_size**() gets size of the memory region
descriptor.

# RETURN VALUE

The **rpma_mr_get_descriptor_size**() function returns 0 on success or a
negative error code on failure. **rpma_mr_get_descriptor_size**() does
not set \*desc_size value on failure.

# ERRORS

**rpma_mr_get_descriptor_size**() can fail with the following error:

-   RPMA_E\_INVAL - mr or desc_size is NULL

# SEE ALSO

**rpma_mr_get_descriptor**(3), **rpma_mr_reg**(3), **librpma**(7) and
https://pmem.io/rpma/
