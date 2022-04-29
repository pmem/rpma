/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * mtt_common-epoll.h -- common epoll functions declarations for MT tests
 */

#ifndef MTT_COMMON_EPOLL
#define MTT_COMMON_EPOLL

#define TIMEOUT_15S (15000) /* [msec] == 15s */

int fd_set_nonblock(int fd);

struct custom_event;

typedef void (*event_func)(struct custom_event *ce);

struct custom_event {
	int fd;
	void *arg;
	event_func func;
};

int epoll_add(int epoll, int fd, void *arg, event_func func, struct custom_event *ce);

void epoll_delete(int epoll, struct custom_event *ce);

#endif /* MTT_COMMON_EPOLL */
