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

#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INIT_TABLE_SIZE 4096
#define F_INCR 3
#define F_THRS 0.65

static table table_default = {
  .ltable = 0,
  .ctable = 0,
  .thrs = 0
};

static unsigned int hash(unsigned int, const char*);
static int resize();
static table_s *colision(unsigned int);
static int colision_add(table_s*, unsigned int, unsigned long, const char*, const char*);
static int add(bool, unsigned int, unsigned long, char*, char*);

static unsigned int hash(const unsigned int ltable, const char* key)
{
  unsigned int h = 2166136261u;
  for (const char *p = key; *p; p++)
  {
    h ^= (unsigned int)(unsigned char)(*p);
    h *= 16777619u;
  }

  return h % ltable;
}

static int resize()
{
  if (!table_default.init) return -1;
  unsigned long oltable = table_default.ltable;
  table_s **os = table_default.s;

  table_default.ltable += F_INCR;
  table_default.thrs = table_default.ltable * F_THRS;
  table_default.s = calloc(table_default.ltable, sizeof(table_s*));
  table_default.ctable = 0;

  for (unsigned int i = 0; i < oltable; i++)
  {
    table_s *s = os[i];
    while (s != NULL)
    {
      if (add(true, s->lkey, s->lvalue, s->key, s->value) != 0) return -1;
      table_s *tso = s->next;
      free(s);
      s = tso;
    }
  }
  return 0;
}

static table_s *colision(const unsigned int h)
{
  if (!table_default.init) return NULL;
  return table_default.s[h];
}

static int colision_add(table_s *ts, const unsigned int lkey, const unsigned long lvalue, const char *key, const char *value)
{
  if (!table_default.init || ts == NULL) return -1;
  while(ts != NULL)
  {
    if (strcmp(ts->key, key) == 0)
    {
      char *ov = ts->value;
      ts->value = malloc(lvalue);
      if (ts->value == NULL)
      {
        ts->value = ov;
        return -1;
      }
      free(ov);
      ts->lvalue = lvalue;
      ts->value = strdup(value);
      return 0;
    }

    if (ts->next == NULL) break;
    ts = ts->next;
  }

  table_s *tsn = malloc(sizeof(table_s));
  tsn->key = strdup(key);
  tsn->value = strdup(value);
  tsn->lkey = lkey;
  tsn->lvalue = lvalue;
  tsn->next = NULL;
  if (ts == NULL)
  {
    free(tsn);
    return -1;
  }
  ts->next = tsn;

  return 0;
}

static int add(const bool slock, const unsigned int lkey, const unsigned long lvalue, char *key, char *value)
{
  table_s *ts;
  int ret = -1;

  if (!table_default.init) return ret;
  if (table_default.ctable >= table_default.thrs)
  {
    if (pthread_rwlock_rdlock(&table_default.rwl) != 0) return ret;
    if (resize() != 0)
    {
      perror("add resize table");
      exit(1);
    }
    if (pthread_rwlock_unlock(&table_default.rwl) != 0)
    {
      perror("add resize table unlock");
      exit(1);
    }
  }

  if (!slock && pthread_rwlock_wrlock(&table_default.rwl) != 0) return ret;
  const unsigned int h = hash(table_default.ltable, key);
  if ((ts = colision(h)) != NULL)
  {
    if (colision_add(ts, lkey, lvalue, key, value) != 0) goto add_end;
  }

  if ((ts = malloc(sizeof(table_s))) == NULL) goto add_end;
  ts->next = NULL;
  if ((ts->key = malloc(lkey)) == NULL)
  {
    free(ts);
    goto add_end;
  }
  if ((ts->value = malloc(lvalue)) == NULL)
  {
    free(ts->key);
    free(ts);
    goto add_end;
  }
  strncpy(ts->key, key, lkey);
  strncpy(ts->value, value, lvalue);
  ts->lkey = lkey;
  ts->lvalue = lvalue;
  table_default.s[h] = ts;
  table_default.ctable++;
  ret = 0;

  add_end:
  if (!slock && pthread_rwlock_unlock(&table_default.rwl) != 0)
  {
    perror("add end table unlock");
    exit(1);
  }

  return ret;
}

int table_del(const char *key)
{
  table_s *s, *p = NULL;
  int rt = -1;

  if (strlen(key) > UINT32_MAX) return -1;
  if (!table_default.init) return rt;
  if (pthread_rwlock_rdlock(&table_default.rwl) != 0) return rt;
  const unsigned int h = hash(table_default.ltable, key);
  if ((s = table_default.s[h]) == NULL) goto table_del_final;

  while (s != NULL)
  {
    if (strcmp(s->key, key) == 0)
    {
      if (p == NULL) table_default.s[h] = s->next;
      else p->next = s->next;
      goto table_del_found;
    }
    p = s;
    s = s->next;
  }
  goto table_del_final;

  table_del_found:
  free(s->key);
  free(s->value);
  free(s);
  rt = 0;

  table_del_final:
  if (pthread_rwlock_unlock(&table_default.rwl) != 0)
  {
    perror("table del unlock");
    exit(EXIT_FAILURE);
  }

  return rt;
}

int table_add(const unsigned int lkey, const unsigned long lvalue, char *key, char *value)
{
  if (strlen(key) != lkey) return -1;
  if (strlen(value) != lvalue) return -1;
  return add(false, lkey, lvalue, key, value);
}

table_s *table_getbk(const char *key)
{
  table_s *s = NULL;

  if (strlen(key) > UINT32_MAX) return NULL;
  if (!table_default.init) return NULL;
  const unsigned int h = hash(table_default.ltable, key);
  if ((s = table_default.s[h]) == NULL) return NULL;

  while (s != NULL)
  {
    if (strcmp(s->key, key) == 0) return s;
    s = s->next;
  }

  return NULL;
}

table *table_setup(void)
{
  if (table_default.init) return &table_default;
  if (pthread_rwlock_init(&table_default.rwl, NULL) != 0) goto table_setup_error;
  if ((table_default.s = calloc(INIT_TABLE_SIZE, sizeof(table_s*))) == NULL) goto table_setup_error;

  table_default.ltable = INIT_TABLE_SIZE;
  table_default.ctable = 0;
  table_default.thrs = table_default.ltable * F_THRS;
  table_default.init = true;

  return &table_default;
  table_setup_error:
  if (table_default.s != NULL) free(table_default.s);
  return NULL;
}