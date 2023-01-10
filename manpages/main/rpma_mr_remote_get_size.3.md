---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_remote_get_size.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.2.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_remote_get_size** - get a remote memory region size

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_get_size(const struct rpma_mr_remote *mr,
                          size_t *size);

# DESCRIPTION

**rpma_mr_remote_get_size**() gets the size of the remote memory region.

# RETURN VALUE

The **rpma_mr_remote_get_size**() function returns 0 on success or a
negative error code on failure. **rpma_mr_remote_get_size**() does not
set \*size value on failure.

# ERRORS

**rpma_mr_remote_get_size**() can fail with the following error:

-   RPMA_E\_INVAL - mr or size is NULL

# SEE ALSO

**rpma_mr_remote_from_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
