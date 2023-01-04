---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_log_set_threshold.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.1.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2022, Intel Corporation)

# NAME

**rpma_log_set_threshold** - set the logging threshold level

# SYNOPSIS

          #include <librpma.h>

          int rpma_log_set_threshold(enum rpma_log_threshold threshold, enum rpma_log_level level);

          enum rpma_log_level {
                  RPMA_LOG_DISABLED,
                  RPMA_LOG_LEVEL_FATAL,
                  RPMA_LOG_LEVEL_ERROR,
                  RPMA_LOG_LEVEL_WARNING,
                  RPMA_LOG_LEVEL_NOTICE,
                  RPMA_LOG_LEVEL_INFO,
                  RPMA_LOG_LEVEL_DEBUG,
          };

          enum rpma_log_threshold {
                  RPMA_LOG_THRESHOLD,
                  RPMA_LOG_THRESHOLD_AUX,
                  RPMA_LOG_THRESHOLD_MAX
          };

# DESCRIPTION

**rpma_log_set_threshold**() sets the logging *threshold* *level*.

Available thresholds are:

-   *RPMA_LOG_THRESHOLD* - the main *threshold* used to filter out
    undesired logging messages. Messages on a higher *level* than the
    primary *threshold* *level* are ignored. *RPMA_LOG_DISABLED* shall
    be used to suppress logging. The default value is RPMA_LOG_WARNING.

-   *RPMA_LOG_THRESHOLD_AUX* - the auxiliary *threshold* intended for
    use inside the logging function (please see
    **rpma_log_get_threshold**(3)). The logging function may or may not
    take this *threshold* into consideration. The default value is
    *RPMA_LOG_DISABLED*.

Available *threshold* levels are defined by enum rpma_log_level:

-   *RPMA_LOG_DISABLED* - all messages will be suppressed

-   *RPMA_LOG_LEVEL_FATAL* - an error that causes the library to stop
    working immediately

-   *RPMA_LOG_LEVEL_ERROR* - an error that causes the library to stop
    working properly

-   *RPMA_LOG_LEVEL_WARNING* - an error that could be handled in the
    upper *level*

-   *RPMA_LOG_LEVEL_NOTICE* - non-massive info mainly related to public
    API function completions e.g. connection established

-   *RPMA_LOG_LEVEL_INFO* - massive info e.g. every write operation
    indication

-   *RPMA_LOG_LEVEL_DEBUG* - debug info e.g. write operation dump

# THE DEFAULT LOGGING FUNCTION

The default logging function writes messages to **syslog**(3) and to
**stderr**(3), where **syslog**(3) is the primary destination
(*RPMA_LOG_THRESHOLD* applies) whereas **stderr**(3) is the secondary
destination (*RPMA_LOG_THRESHOLD_AUX* applies).

# RETURN VALUE

**rpma_log_syslog_set_threshold**() function returns 0 on success or a
negative error code on failure.

# ERRORS

**rpma_log_set_threshold**() can fail with the following errors:

-   RPMA_E\_INVAL - *threshold* is not *RPMA_LOG_THRESHOLD* nor
    *RPMA_LOG_THRESHOLD_AUX*

-   RPMA_E\_INVAL - *level* is not a value defined by enum
    rpma_log_level type

-   RPMA_E\_AGAIN - a temporary error occurred, the retry may fix the
    problem

# SEE ALSO

**rpma_log_get_threshold**(3), **rpma_log_set_function**(3),
**librpma**(7) and https://pmem.io/rpma/
