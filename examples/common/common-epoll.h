/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * common-epoll.h -- common epoll functions declarations for examples
 */

#ifndef EXAMPLES_COMMON_EPOLL
#define EXAMPLES_COMMON_EPOLL

int fd_set_nonblock(int fd);

struct custom_event;

typedef void (*event_func)(struct custom_event *ce);

struct custom_event {
	int fd;
	void *arg;
	event_func func;
};

int epoll_add(int epoll, int fd, void *arg, event_func func,
		struct custom_event *ce);

void epoll_delete(int epoll, struct custom_event *ce);

#endif /* EXAMPLES_COMMON_EPOLL */
