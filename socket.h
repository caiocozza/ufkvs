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
#ifndef SOCKET_H
#define SOCKET_H

#include "error.h"
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>

static inline int socket_set_nblocking(const int fd)
{
  int flags;

  flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) goto socket_snb_error;
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) goto socket_snb_error;
  return flags;

  socket_snb_error:
  perror("(socket) fcntl");
  return GENERIC_CMN_ERROR;
}

static inline int socket_new()
{
  int listenfd, flags;

  if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("(socket) socket");
    return GENERIC_CMN_ERROR;
  }

  if ((flags = socket_set_nblocking(listenfd)) < 0)
  {
    perror("(socket) socket_set_nblocking");
    return GENERIC_CMN_ERROR;
  }

  return listenfd;
}

#endif //SOCKET_H