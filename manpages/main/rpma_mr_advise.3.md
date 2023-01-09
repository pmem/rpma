---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_mr_advise.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.1"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_mr_advise** - give advice about an address range in a memory
registration

# SYNOPSIS

          #include <librpma.h>

          struct rpma_mr_local *mr;
          int rpma_mr_advise(struct rpma_mr_local *mr, size_t offset, size_t len, int advice,
                  uint32_t flags);

# DESCRIPTION

**rpma_mr_advise**() gives advice about an address range in a memory
registration. The usage parameter specifies the operations that can be
performed on the given memory address range. For available advice and
flags values please see **ibv_advise_mr**(3).

# RETURN VALUE

The **rpma_mr_advise**() function returns 0 on success or a negative
error code on failure.

# ERRORS

**rpma_mr_advise**() can fail with the following errors:

-   RPMA_E\_INVAL - in one of the following:

    -   the requested range is out of the memory registration bounds

    -   the memory registration usage does not allow the specific advice

    -   the flags are invalid

-   RPMA_E\_NOSUPP - the operation is not supported by the system

-   RPMA_E\_PROVIDER - **ibv_mr_advise**(3) failed for other errors

# SEE ALSO

**rpma_mr_reg**(3), **ibv_mr_advise**(3), **librpma**(7) and
https://pmem.io/rpma/
