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

#include "worker.h"
#if defined(__linux__)
#include "epoll.h"
#elif defined(__APPLE__)
// TODO
#endif

static int sfd = -1;

static void worker_input_handler(int fd)
{
  // will decode commands and process
}

static void worker_output_handler(void)
{
  // will data output to clients
}

static void *worker_fn(void *args)
{
  if (sfd < 0)
  {
    perror("(worker) sfd not set");
    return NULL;
  }

  printf("(worker) status: live\n");
  bool die = false;
#if defined(__linux__)
  if (epoll_loop(
      -1,
      sfd,
      NULL,
      &worker_input_handler,
      &worker_output_handler,
      &die
    ) < 0)
  {
    // TODO: handle
  }
#elif defined(__APPLE__)
// TODO
#endif
}

int worker_setup(int fd, pthread_t (*workers)[WORKERS])
{
  sfd = fd;
  for (unsigned int iw = 0; iw < WORKERS; ++iw)
    if (pthread_create(&(*workers)[iw], NULL, worker_fn, NULL) < 0)
    {
      perror("(worker) pthread_create");
      return -1;
    }
  return 0;
}