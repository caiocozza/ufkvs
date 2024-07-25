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

#include "epoll.h"

#define UGKV_MAXEVENTS 512

int epoll_loop(const int listenfd, const int epollfd, bool *die)
{
  int listfd;
  struct epoll_event events[UGKV_MAXEVENTS];

  while (!*die)
  {
    if ((listfd = epoll_wait(epollfd, events, UGKV_MAXEVENTS, -1)) < 0)
    {
      perror("(epoll) epoll_wait");
      return -1;
    }

    for (unsigned int ifd = 0; ifd < listfd; ++ifd)
    {
      // handle new connection
      if (events[ifd].data.fd == listenfd)
      {

      } else if (events[ifd].events & EPOLLIN) // handle input data
      {

      } else if (events[ifd].events & EPOLLOUT) // handle output data
      {

      } else if (events[ifd].events & EPOLLERR) // handle error
      {
        // requires closing
      } else if (events[ifd].events & EPOLLHUP) // handle not ready to read state
      {
        // requires closing
      }
    }
  }

  return 0;
}