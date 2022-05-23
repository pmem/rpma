// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * common-hello.c -- a hello message structure declarations
 */


#define LANG_NUM	(sizeof(hello_str) / sizeof(hello_str[0]))

#include <stdio.h>
#include <string.h>
#include "common-hello.h"

static const char *hello_str[] = {
	[en] = "Hello world!",
	[es] = "¡Hola Mundo!"
};

void
write_hello_str(struct hello_t *hello, enum lang_t lang)
{
	hello->lang = lang;
	strncpy(hello->str, hello_str[hello->lang], HELLO_STR_SIZE - 1);
	hello->str[HELLO_STR_SIZE - 1] = '\0';
}

void
translate(struct hello_t *hello)
{
	printf("translating...\n");
	enum lang_t lang = (enum lang_t)((hello->lang + 1) % LANG_NUM);
	write_hello_str(hello, lang);
}
