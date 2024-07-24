// Copyright (c) 2024, Caio Cozza
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef EPOLL_H
#define EPOLL_H

#include "error.h"
#include <sys/epoll.h>
#include <stdio.h>
#include <stdbool.h>

static inline int epoll_new()
{
  int epollfd;

  if ((epollfd = epoll_create1(0)) < 0)
  {
    perror("(epoll) epoll_create1");
    return GENERIC_CMN_ERROR;
  }

  return epollfd;
}

static inline int epoll_inadd(const int epollfd, const int fd)
{
  struct epoll_event events;
  events.data.fd = fd;
  events.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &events) < 0)
  {
    perror("(epoll) epoll_ctl");
    return -1;
  }

  return 0;
}

int epoll_loop(bool*);

#endif //EPOLL_H