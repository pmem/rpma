// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * common-epoll.c -- common epoll functions for examples
 */

#include <stdio.h>
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
	if (ret < 0) {
		perror("fcntl");
		return errno;
	}

	int flags = ret | O_NONBLOCK;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret < 0) {
		perror("fcntl");
		return errno;
	}

	return 0;
}

/*
 * epoll_add -- add a custom event to the epoll
 */
int
epoll_add(int epoll, int fd, void *arg, event_func func,
		struct custom_event *ce)
{
	/* set O_NONBLOCK flag for the provided fd */
	int ret = fd_set_nonblock(fd);
	if (ret)
		return -1;

	/* prepare a custom event structure */
	ce->fd = fd;
	ce->arg = arg;
	ce->func = func;

	/* prepare an epoll event */
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = ce;

	/* add the event to epoll */
	if (epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &event))
		return errno;

	return 0;
}

/*
 * epoll_delete -- remove the custom event from the epoll
 */
void
epoll_delete(int epoll, struct custom_event *ce)
{
	(void) epoll_ctl(epoll, EPOLL_CTL_DEL, ce->fd, NULL);
	ce->fd = -1;
}
