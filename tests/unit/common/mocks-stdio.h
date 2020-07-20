/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-stdio.h -- the stdio mocks' header
 */

#ifndef MOCKS_STDIO_H
#define MOCKS_STDIO_H

#include <stdio.h>

#define MOCK_BUFF_LEN		1024

#define MOCK_STDIO_ERROR	(-1)

int __wrap_vsnprintf(char *str, size_t size, const char *format, va_list ap);

int __wrap_snprintf(char *str, size_t size, const char *format, ...);

int __wrap_fprintf(FILE *stream, const char *format, ...);

int __real_vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif /* MOCKS_STDIO_H */
