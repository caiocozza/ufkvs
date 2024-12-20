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
#include "client.h"
#include "processor.h"

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

static void server_connection_handler(void)
{
  int fd;
  if ((fd = socket_accept(server.lfd)) < 0)
  {
    perror("(server) socket_accept");
    return;
  }

  printf("(server) sock on: %d\n", fd);
#if defined (__linux__)
  if (epoll_inadd(server.wfd, fd, true) < 0)
  {
    perror("(server) epoll_inadd");
    close(fd);
    return;
  }

  if (socket_set_nblocking(fd) < 0)
  {
    if (epoll_delete(server.wfd, fd) < 0) perror("(server) epoll_delete");
    close(fd);
    if (client_clear(fd) < 0) perror("(server) client_clear");
    return;
  }
#elif defined(__APPLE__)
#endif
  if(client_set(fd) < 0)
  {
    perror("(server) client_set");
    close(fd);
  }
}

static void server_output_handler(void)
{
}

static void server_input_handler(int fd)
{
}

static int server_setup_inet(void)
{
  struct sockaddr_in addrs;

  if ((server.lfd = socket_new()) < 0) return -1;

  memset(&addrs, 0, sizeof(addrs));
  addrs.sin_family = PF_INET;
  addrs.sin_addr.s_addr = INADDR_ANY;
  addrs.sin_port = htons(server.port);

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
    perror("(server) sfd epoll_new");
    return -1;
  }
  if ((epoll_inadd(server.sfd, server.lfd, false)) < 0)
  {
    perror("(server) sfd epoll_inadd");
    close(server.sfd);
    close(server.lfd);
    return -1;
  }

  if ((server.wfd = epoll_new()) < 0)
  {
    perror("(server) wfd epoll_new");
    close(server.sfd);
    close(server.lfd);
    return -1;
  }

  if (socket_set_nblocking(server.wfd) < 0)
  {
    perror("(server) wfd socket_set_nblocking");
    close(server.wfd);
    close(server.sfd);
    close(server.lfd);
    return -1;
  }

  if (processor_setup_workers() < 0)
  {
    close(server.wfd);
    close(server.sfd);
    close(server.lfd);
    return -1;
  }

  // here is not sfd but a new one we will create
  if (worker_setup(server.wfd, &server.workers) < 0)
  {
    // TODO: handle threads must clean and die
  }

  printf("(server) starting mainloop\n");
  if (epoll_loop(
      server.lfd,
      server.sfd,
      &server_connection_handler,
      &server_input_handler,
      &server_output_handler,
      &server.die
    ) < 0)
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
