// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * common.c -- a common functions used by examples
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/epoll.h>

#include "common-epoll.h"

/*
 * fd_set_nonblock -- set O_NONBLOCK flag for provided file descriptor
 */
int
fd_set_nonblock(int fd)
{
	int ret = fcntl(fd, F_GETFL);
	if (ret < 0)
		return errno;

	int flags = ret | O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret < 0)
		return errno;

	return 0;
}

/*
 * custom_event_set -- fill the custom event fields
 */
void
custom_event_set(struct custom_event *ce, int fd, void *arg,
		event_func func)
{
	ce->fd = fd;
	ce->arg = arg;
	ce->func = func;
}

/*
 * epoll_add -- add a custom event to the epoll
 */
int
epoll_add(int epoll, struct custom_event *ce)
{
	/* assign event to an epoll */
	ce->epoll = epoll;

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = ce;

	if (epoll_ctl(epoll, EPOLL_CTL_ADD, ce->fd, &event))
		return errno;

	return 0;
}

/*
 * epoll_delete -- remove the custom event from the epoll
 */
int
epoll_delete(int epoll, struct custom_event *ce)
{
	if (epoll_ctl(epoll, EPOLL_CTL_DEL, ce->fd, NULL))
		return errno;

	return 0;
}
