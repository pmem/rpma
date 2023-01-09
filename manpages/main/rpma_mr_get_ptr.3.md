---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_get_ptr.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_get_ptr** - get the pointer to the local memory region

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_get_ptr(const struct rpma_mr_local *mr, void **ptr);

# DESCRIPTION

**rpma_mr_get_ptr**() gets a memory region pointer from the local memory
registration object.

# RETURN VALUE

The **rpma_mr_get_ptr**() function returns 0 on success or a negative
error code on failure. **rpma_mr_get_ptr**() does not set \*ptr value on
failure.

# ERRORS

**rpma_mr_get_ptr**() can fail with the following error:

-   RPMA_E\_INVAL - mr or ptr is NULL

# SEE ALSO

**rpma_mr_reg**(3), **librpma**(7) and https://pmem.io/rpma/
