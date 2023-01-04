---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_log_get_threshold.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_log_get_threshold** - get the logging threshold level

# SYNOPSIS

          #include <librpma.h>

          int rpma_log_get_threshold(enum rpma_log_threshold threshold, enum rpma_log_level *level);

# DESCRIPTION

**rpma_log_get_threshold**() gets the current *level* of the
*threshold*. See **rpma_log_set_threshold**(3) for available thresholds
and levels.

# RETURN VALUE

**rpma_log_get_threshold**() function returns 0 on success or a negative
error code on failure.

# ERRORS

**rpma_log_get_threshold**() can fail with the following errors:

-   RPMA_E\_INVAL - *threshold* is not RPMA_LOG_THRESHOLD nor
    RPMA_LOG_THRESHOLD_AUX

-   RPMA_E\_INVAL - \**level* is NULL

# SEE ALSO

**rpma_log_set_function**(3), **rpma_log_set_threshold**(3),
**librpma**(7) and https://pmem.io/rpma/
