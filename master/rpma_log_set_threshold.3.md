---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_log\_set\_threshold** - set the logging threshold level

SYNOPSIS
========

          #include <librpma.h>

          int rpma_log_set_threshold(enum rpma_log_threshold threshold,
                          enum rpma_log_level level);

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

DESCRIPTION
===========

**rpma\_log\_set\_threshold**() sets the logging *threshold* *level*.

Available thresholds are:

-   *RPMA\_LOG\_THRESHOLD* - the main *threshold* used to filter out
    undesired logging messages. Messages on a higher *level* than the
    primary *threshold* *level* are ignored. *RPMA\_LOG\_DISABLED* shall
    be used to suppress logging. The default value is
    RPMA\_LOG\_WARNING.

-   *RPMA\_LOG\_THRESHOLD\_AUX* - the auxiliary *threshold* intended for
    use inside the logging function (please see
    **rpma\_log\_get\_threshold**(3)). The logging function may or may
    not take this *threshold* into consideration. The default value is
    *RPMA\_LOG\_DISABLED*.

Available *threshold* levels are defined by enum rpma\_log\_level:

-   *RPMA\_LOG\_DISABLED* - all messages will be suppressed

-   *RPMA\_LOG\_LEVEL\_FATAL* - an error that causes the library to stop
    working immediately

-   *RPMA\_LOG\_LEVEL\_ERROR* - an error that causes the library to stop
    working properly

-   *RPMA\_LOG\_LEVEL\_WARNING* - an error that could be handled in the
    upper *level*

-   *RPMA\_LOG\_LEVEL\_NOTICE* - non-massive info mainly related to
    public API function completions e.g. connection established

-   *RPMA\_LOG\_LEVEL\_INFO* - massive info e.g. every write operation
    indication

-   *RPMA\_LOG\_LEVEL\_DEBUG* - debug info e.g. write operation dump

THE DEFAULT LOGGING FUNCTION
============================

The default logging function writes messages to **syslog**(3) and to
**stderr**(3), where **syslog**(3) is the primary destination
(*RPMA\_LOG\_THRESHOLD* applies) whereas **stderr**(3) is the secondary
destination (*RPMA\_LOG\_THRESHOLD\_AUX* applies).

RETURN VALUE
============

**rpma\_log\_syslog\_set\_threshold**() function returns 0 on success or
a negative error code on failure.

ERRORS
======

**rpma\_log\_set\_threshold**() can fail with the following errors:

-   RPMA\_E\_INVAL - *threshold* is not *RPMA\_LOG\_THRESHOLD* nor
    *RPMA\_LOG\_THRESHOLD\_AUX*

-   RPMA\_E\_INVAL - *level* is not a value defined by enum
    rpma\_log\_level type

-   RPMA\_E\_AGAIN - a temporary error occurred, the retry may fix the
    problem

SEE ALSO
========

**rpma\_log\_get\_threshold**(3), **rpma\_log\_set\_function**(3),
**librpma**(7) and https://pmem.io/rpma/
