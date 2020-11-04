/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * hello.h -- a hello message structure declarations
 */

#ifndef EXAMPLES_HELLO_H
#define EXAMPLES_HELLO_H

#define HELLO_STR_SIZE 1024

enum lang_t {en, es};

struct hello_t {
	enum lang_t lang;
	char str[HELLO_STR_SIZE];
};

#define HELLO_STR_OFFSET offsetof(struct hello_t, str)
#define HELLO_T_SIZE (sizeof(struct hello_t))

#ifndef RPMA_INTEGRATION_TEST
static const char *hello_str[] = {
	[en] = "Hello world!",
	[es] = "¡Hola Mundo!"
};

#define LANG_NUM	(sizeof(hello_str) / sizeof(hello_str[0]))

#endif /* RPMA_INTEGRATION_TEST */

#endif /* EXAMPLES_HELLO_H */
