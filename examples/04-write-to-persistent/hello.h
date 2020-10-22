/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * hello.h -- a hello message structure declarations
 */

#ifndef HELLO_H
#define HELLO_H

#include <stdlib.h>
#include <stdio.h>

#define HELLO_STR_SIZE 1024

enum lang_t {en, es};

struct hello_t {
	enum lang_t lang;
	char str[HELLO_STR_SIZE];
};

#define HELLO_STR_OFFSET offsetof(struct hello_t, str)
#endif
