---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_dereg.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_dereg** - delete a local memory registration object

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_local;
          int rpma_mr_dereg(struct rpma_mr_local **mr_ptr);

# DESCRIPTION

**rpma_mr_dereg**() deregisters a memory region and deletes a local
memory registration object.

# RETURN VALUE

The **rpma_mr_dereg**() function returns 0 on success or a negative
error code on failure. **rpma_mr_dereg**() does not set \*mr_ptr value
to NULL on failure.

# ERRORS

**rpma_mr_dereg**() can fail with the following errors:

-   RPMA_E\_INVAL - mr_ptr is NULL

-   RPMA_E\_PROVIDER - memory deregistration failed

# SEE ALSO

**rpma_mr_reg**(3), **librpma**(7) and https://pmem.io/rpma/
