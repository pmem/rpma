---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_remote_delete.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.3.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_remote_delete** - delete the remote memory region\'s structure

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_remote;
          int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);

# DESCRIPTION

**rpma_mr_remote_delete**() deletes the remote memory region\'s
structure.

# RETURN VALUE

The **rpma_mr_remote_delete**() function returns 0 on success or a
negative error code on failure. **rpma_mr_remote_delete**() does not set
\*mr_ptr value to NULL on failure.

# ERRORS

**rpma_mr_remote_delete**() can fail with the following error:

-   RPMA_E\_INVAL - mr_ptr is NULL

# SEE ALSO

**rpma_mr_remote_from_descriptor**(3), **librpma**(7) and
https://pmem.io/rpma/
