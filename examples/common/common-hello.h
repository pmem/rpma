/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-hello.h -- a hello message structure declarations
 */

#ifndef COMMON_HELLO_H
#define COMMON_HELLO_H

#define HELLO_STR_SIZE 1024

#define KILOBYTE 1024

enum lang_t {en, es};

struct hello_t {
	enum lang_t lang;
	char str[HELLO_STR_SIZE];
};

#define HELLO_STR_OFFSET offsetof(struct hello_t, str)
#define HELLO_T_SIZE (sizeof(struct hello_t))

#ifndef RPMA_INTEGRATION_TEST

extern const char *hello_str[];

#define LANG_NUM	(sizeof(hello_str) / sizeof(hello_str[0]))

#endif /* RPMA_INTEGRATION_TEST */

void write_hello_str(struct hello_t *hello, enum lang_t lang);

void translate(struct hello_t *hello);

#endif /* COMMON_HELLO_H */
