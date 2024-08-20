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
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //tmp

static processor_t proc = {
  .cnd  = PTHREAD_COND_INITIALIZER,
  .mtx  = PTHREAD_MUTEX_INITIALIZER,
  .head = NULL,
  .tail = NULL
};

static int processor_get(const processor_item_t *item)
{
  unsigned int keysize;

  memcpy(&keysize, item->data, 4);

  char *key = malloc(keysize);
  if (key == NULL) return -1;

  memcpy(key, item->data + 4, keysize);

  const table_s *found = table_getbk(key);
  if (found == NULL)
  {
    free(key);
    return -1;
  }

  write(item->fd, found->value, found->lvalue);
  free(key);

  return 0;
}

static int processor_set(const processor_item_t *item)
{
  unsigned int keysize, valuesize;

  memcpy(&keysize, item->data, 4);
  memcpy(&valuesize, item->data + 4, 4);

  char *key = malloc(keysize);
  if (key == NULL) return -1;

  char *value = malloc(valuesize);
  if (value == NULL)
  {
    free(key);
    return -1;
  }

  memcpy(key, item->data + 8, keysize);
  memcpy(value, item->data + 8 + keysize, valuesize);

  if (table_add(keysize, valuesize, key, value) < 0)
  {
    free(key);
    free(value);
    return -1;
  }

  write(item->fd, value, valuesize);

  free(key);
  free(value);
  return 0;
}

static int processor_exec(const processor_item_t *item)
{
  int ret;

  // distinguish data here
  switch (item->cmd)
  {
  case 1:
    return processor_set(item);
  case 2:
    return processor_get(item);
  default:
    return -1;
  }
}

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
    processor_exec(item);
    // -------

    // after processing we can free the proc item
    free(item->data);
    free(item);
  }
}

int processsor_enqueue(int fd, unsigned int size, unsigned int id, unsigned short cmd, const char* data)
{
  processor_item_t *item = malloc(sizeof(processor_item_t));
  if (item == NULL) return -1;
  
  item->data = malloc(size);
  if (item->data == NULL)
  {
    free(item);
    return -1;
  }

  memcpy(item->data, data, size);
  item->fd = fd;
  item->cmd = cmd;
  item->id = id;
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

int processor_setup_workers(void)
{
  if (table_setup() == NULL) return -1;
  for (unsigned int iw = 0; iw < PROCESSOR_WORKERS; ++iw)
    if (pthread_create(&proc.workers[iw], NULL, processor_worker_fn, NULL) < 0)
    {
      perror("(processor) pthread_create");
      return -1;
    }
  return 0;
}