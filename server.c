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

#include "server.h"
#include "socket.h"

#if defined (__linux__)
#include "epoll.h"
#elif defined(__APPLE__)
// TODO
#endif

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

static server_t server = {
  .port = 8080
};

static int server_setup_inet(void)
{
  struct sockaddr_in addrs;

  if ((server.lfd = socket_new()) < 0) return -1;

  memset(&addrs, 0, sizeof(addrs));
  addrs.sin_family = PF_INET;
  addrs.sin_addr.s_addr = INADDR_ANY;
  addrs.sin_port = server.port;

  if (bind(server.lfd, (struct sockaddr*)&addrs, sizeof(addrs)) < 0)
  {
    perror("(server) bind");
    goto server_linet_error;
  }
  if (listen(server.lfd, SOMAXCONN) < 0)
  {
    perror("(server) listen");
    goto server_linet_error;
  }

  return 0;

  server_linet_error:
  close(server.lfd);
  return -1;
}

int server_start(void)
{
  if (server_setup_inet() < 0)
  {
    perror("(server) server_setup_inet");
    return -1;
  }
#if defined (__linux__)
  if ((server.sfd = epoll_new()) < 0)
  {
    perror("(server) epoll_new");
    return -1;
  }
  if ((epoll_inadd(server.sfd, server.lfd)) < 0)
  {
    perror("(server) epoll_inadd");
    close(server.sfd);
    close(server.lfd);
    return -1;
  }

  if (epoll_loop(server.lfd, server.sfd, &server.die) < 0)
  {
    perror("(server) epoll_loop");
    close(server.sfd);
    close(server.lfd);
    return -1;
  }
#elif defined(__APPLE__)
// TODO
#endif
}
