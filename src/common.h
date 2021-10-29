/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * common.h -- librpma common internal definitions
 */

#ifndef LIBRPMA_COMMON_H
#define LIBRPMA_COMMON_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define CLIP_TO_INT(size)	((size) > INT_MAX ? INT_MAX : (int)(size))

#define OP_CONTEXT_MAX	0x7FFFFFFFFFFFFFFF
#define OP_CONTEXT_HIGHEST_BIT	0x8000000000000000

#define CHECK_MAX(op_context)	((uint64_t)op_context > OP_CONTEXT_MAX)
#define SET_HIGHEST_BIT(op_context)	\
		((uint64_t)(op_context) | OP_CONTEXT_HIGHEST_BIT)

#ifdef __GNUC__
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif

#endif /* LIBRPMA_COMMON_H */
