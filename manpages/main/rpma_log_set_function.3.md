---
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\"https://github.com/pmem\">GitHub repositories</a> are BSD-licensed open source."
aliases: ["rpma_log_set_function.3.html"]
title: "librpma | PMDK"
header: "librpma API version 1.2.0"
---
{{< manpages >}}

[comment]: <> (SPDX-License-Identifier: BSD-3-Clause)
[comment]: <> (Copyright 2020-2023, Intel Corporation)

# NAME

**rpma_log_set_function** - set the logging function

# SYNOPSIS

          #include <librpma.h>

          typedef void rpma_log_function(
                  enum rpma_log_level level,
                  const char *file_name,
                  const int line_no,
                  const char *function_name,
                  const char *message_format,
                  ...);

          int rpma_log_set_function(rpma_log_function *log_function);

# DESCRIPTION

**rpma_log_set_function**() allows choosing the function which will get
all the generated logging messages. The *log_function* can be either
RPMA_LOG_USE_DEFAULT_FUNCTION which will use the default logging
function (built into the library) or a pointer to a user-defined
function.

Parameters of a user-defined log function are as follow:

-   *level* - the log *level* of the message

-   *file_name* - name of the source file where the message coming from.
    It could be set to NULL and in such case neither *line_no* nor
    *function_name* are provided.

-   *line_no* - the source file line where the message coming from

-   *function_name* - the function name where the message coming from

-   *message_format* - **printf**(3)**-like** format string of the
    message

-   \"\...\" - additional arguments of the message format string

# THE DEFAULT LOGGING FUNCTION

The initial value of the logging function is
RPMA_LOG_USE_DEFAULT_FUNCTION. This function writes messages to
**syslog**(3) (the primary destination) and to **stderr**(3) (the
secondary destination).

# RETURN VALUE

**rpma_log_set_function**() function returns 0 on success or error code
on failure.

# ERRORS

-   RPMA_E\_AGAIN - a temporary error occurred, the retry may fix the
    problem

# NOTE

The logging messages on the levels above the RPMA_LOG_THRESHOLD *level*
won\'t trigger the logging function.

The user defined function must be thread-safe.

# SEE ALSO

**rpma_log_get_threshold**(3), **rpma_log_set_threshold**(3),
**librpma**(7) and https://pmem.io/rpma/
