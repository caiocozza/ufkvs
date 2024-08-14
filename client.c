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
#include "client.h"
#include "processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define CLIENTS 8192

static client_t clients[CLIENTS];

int client_clear(const int fd)
{
  if (fd > CLIENTS) return -1;
  if (clients[fd].fd < 0) return -1;

  printf("(client) clear socket: %d\n", fd);
  clients[fd].fd = -1;
  clients[fd].locked = false;
  clients[fd].buffersize = 0;
  if (clients->buffer != NULL) free(clients[fd].buffer);
  if (pthread_mutex_destroy(&clients[fd].mtx) < 0) perror("(client) pthread_mutex_destroy");
  
  return 0;
}

int client_set(const int fd)
{
  if (fd > CLIENTS) return -1;
  if (clients[fd].fd > -1) return -1;

  clients[fd].fd = fd;
  clients[fd].locked = false;
  clients[fd].buffer = NULL;
  clients[fd].buffersize = 0;
  if (pthread_mutex_init(&clients[fd].mtx, NULL) < 0)
  {
    perror("(client) pthread_mutex_init");
    clients[fd].fd = -1;
    return -1;
  }

  return 0;
}

int client_append(const int fd, const char *data, const size_t datasize)
{
  unsigned int messagesize = 0, messageid = 0;
  unsigned short messagecmd = 0;

  if (fd > CLIENTS) return -1;
  if (clients[fd].fd < 0) return -1;

  if (pthread_mutex_lock(&clients[fd].mtx) < 0)
  {
    perror("(client) pthread_mutex_lock");
    return -1;
  }

  clients[fd].locked = true;

  if (clients[fd].buffer == NULL)
    if ((clients[fd].buffer = malloc(datasize)) == NULL)
    {
      perror("(client) malloc");
      return -1;
    }
  else
  {
    char *tmpbuffer = realloc(clients[fd].buffer, clients[fd].buffersize + datasize);
    if (tmpbuffer == NULL)
    {
      perror("(client) realloc");
      return client_clear(fd);
    }
  }

  memcpy(clients[fd].buffer + clients[fd].buffersize, data, datasize);
  clients[fd].buffersize += datasize;

  // check for a full message
  // if we have -> put it in the execution queue / another worker will handle
  // free the buffer here and unlock mutex.

  // if message is not ready
  // unlock mtx

  while (clients[fd].buffersize > 10)
  {
    memcpy(&messagesize, clients[fd].buffer, 4);
    if (messagesize > (clients[fd].buffersize - 10)) goto client_unlock;

    memcpy(&messagecmd, clients[fd].buffer + 4, 2);
    memcpy(&messageid, clients[fd].buffer + 6, 4);

    char data[messagesize];
    memcpy(&data, clients[fd].buffer + 10, messagesize);

    // dispatch message here
    //write(fd, message, messagesize);
    if (processsor_enqueue(fd, messagesize, messageid, messagecmd, data) < 0)
    {
      exit(EXIT_FAILURE);
    }
    // ...
    // now free
    int diff = clients[fd].buffersize - (messagesize + 10);
    if (diff > 0)
    {
      char *tmpbuffer = malloc(diff);
      free(clients[fd].buffer);
      clients[fd].buffer = tmpbuffer;
      clients[fd].buffersize = diff;
    } else
    {
      free(clients[fd].buffer);
      clients[fd].buffer = NULL;
      clients[fd].buffersize = 0;
    }

    messagesize = 0;
    messagecmd = 0;
    messageid = 0;
  }

  client_unlock:
  if (pthread_mutex_unlock(&clients[fd].mtx) < 0)
  {
    perror("(client) pthread_mutex_unlock");
    return client_clear(fd);
  }

  return 0;
}

void client_setup(void)
{
  for(unsigned int ic = 0; ic < CLIENTS; ++ic)
  {
    clients[ic].fd = -1;
    clients[ic].locked = false;
    clients[ic].buffer = NULL;
  }
}