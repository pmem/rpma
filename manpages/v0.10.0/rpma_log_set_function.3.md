---
layout: manual
Content-Style: 'text/css'
title: LIBRPMA
collection: librpma
date: rpma API version 0.9.0
...

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020, Intel Corporation)

NAME
====

**rpma\_log\_set\_function** - set the logging function

SYNOPSIS
========

          #include <librpma.h>

          typedef void rpma_log_function(
                  enum rpma_log_level level,
                  const char *file_name,
                  const int line_no,
                  const char *function_name,
                  const char *message_format,
                  ...);

          int rpma_log_set_function(rpma_log_function *log_function);

DESCRIPTION
===========

**rpma\_log\_set\_function**() allows choosing the function which will
get all the generated logging messages. The *log\_function* can be
either RPMA\_LOG\_USE\_DEFAULT\_FUNCTION which will use the default
logging function (built into the library) or a pointer to a user-defined
function.

Parameters of a user-defined log function are as follow:

-   *level* - the log *level* of the message

-   *file\_name* - name of the source file where the message coming
    from. It could be set to NULL and in such case neither *line\_no*
    nor *function\_name* are provided.

-   *line\_no* - the source file line where the message coming from

-   *function\_name* - the function name where the message coming from

-   *message\_format* - **printf**(3)**-like** format string of the
    message

-   \"\...\" - additional arguments of the message format string

THE DEFAULT LOGGING FUNCTION
============================

The initial value of the logging function is
RPMA\_LOG\_USE\_DEFAULT\_FUNCTION. This function writes messages to
**syslog**(3) (the primary destination) and to **stderr**(3) (the
secondary destination).

RETURN VALUE
============

**rpma\_log\_set\_function**() function returns 0 on success or error
code on failure.

ERRORS
======

-   RPMA\_E\_AGAIN - a temporary error occurred, the retry may fix the
    problem

NOTE
====

The logging messages on the levels above the RPMA\_LOG\_THRESHOLD
*level* won\'t trigger the logging function.

The user defined function must be thread-safe.

SEE ALSO
========

**rpma\_log\_get\_threshold**(3), **rpma\_log\_set\_threshold**(3),
**librpma**(7) and https://pmem.io/rpma/
