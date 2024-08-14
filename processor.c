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
#include "processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //tmp

static processor_t proc = {
  .cnd  = PTHREAD_COND_INITIALIZER,
  .mtx  = PTHREAD_MUTEX_INITIALIZER,
  .head = NULL,
  .tail = NULL,
  .ofd  = -1
};

static void *processor_worker_fn(void *args)
{
  while(1)
  {
    if (pthread_mutex_lock(&proc.mtx) < 0)
    {
      perror("(processor) worker_fn pthread_mutex_lock");
      exit(EXIT_FAILURE);
    }

    while (proc.head == NULL)
      if (pthread_cond_wait(&proc.cnd, &proc.mtx) < 0)
      {
        perror("(processor) worker_fn pthread_cond_wait");
        exit(EXIT_FAILURE);
      }
    
    processor_item_t *item = proc.head;
    proc.head = item->nxt;

    if (pthread_mutex_unlock(&proc.mtx) < 0)
    {
      perror("(processor) pthread_mutex_unlock");
      exit(EXIT_FAILURE);     
    }
    // free to proc
    write(item->fd, item->data, item->size); // TODO: tmp, must use the ofd (output)
    // -------

    // after processing we can free the proc item
    free(item->data);
    free(item);
  }
}

int processsor_enqueue(int fd, unsigned int size, const char* data)
{
  if (proc.ofd == -1) return -1;
  processor_item_t *item = malloc(sizeof(processor_item_t));
  if (item == NULL) return -1;
  
  item->data = strdup(data);
  item->fd = fd;
  item->size = size;
  item->nxt = NULL;

  if (pthread_mutex_lock(&proc.mtx) < 0) goto processor_enqueue_error;

  if (proc.head == NULL)
  {
    proc.head = item;
    proc.tail = item;
    goto processor_enqueue_final;
  }

  if (proc.tail != NULL)
  {
    proc.tail->nxt = item;
    proc.tail = item;
    goto processor_enqueue_final;
  }

  if (pthread_mutex_unlock(&proc.mtx) < 0)
  {
    perror("(processor) processor_enqueue pthread_mutex_unlock");
    exit(EXIT_FAILURE);
  }

  processor_enqueue_error:
  free(item->data);
  free(item);
  return -1;

  processor_enqueue_final:
  if (pthread_cond_signal(&proc.cnd) < 0)
  {
    perror("(processor) processor_enqueue pthread_cond_signal");
    goto processor_enqueue_error;
  }
  if (pthread_mutex_unlock(&proc.mtx) < 0)
  {
    perror("(processor) processor_enqueue pthread_mutex_unlock");
    goto processor_enqueue_error;
  }
  return 0;
}

int processor_setup_workers(int ofd)
{
  proc.ofd = ofd;
  for (unsigned int iw = 0; iw < PROCESSOR_WORKERS; ++iw)
    if (pthread_create(&proc.workers[iw], NULL, processor_worker_fn, NULL) < 0)
    {
      perror("(processor) pthread_create");
      return -1;
    }
  return 0;
}