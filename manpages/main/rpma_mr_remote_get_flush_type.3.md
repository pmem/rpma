---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_remote_get_flush_type.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_remote_get_flush_type** - get a remote memory region\'s flush
types

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr, int *flush_type);

# DESCRIPTION

**rpma_mr_remote_get_flush_type**() gets flush types supported by the
remote memory region.

# RETURN VALUE

The **rpma_mr_remote_get_flush_type**() function returns 0 on success or
a negative error code on failure. **rpma_mr_remote_get_flush_type**()
does not set \*flush_type value on failure.

# ERRORS

**rpma_mr_remote_get_flush_type**() can fail with the following error:

-   RPMA_E\_INVAL - mr or flush_type is NULL

# SEE ALSO

**rpma_mr_remote_from_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
