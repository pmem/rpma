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
#endif
