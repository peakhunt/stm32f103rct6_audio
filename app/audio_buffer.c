//
// read this
// https://www.kernel.org/doc/html/latest/process/volatile-considered-harmful.html
//
#include "audio_buffer.h"

static struct list_head    _free_list;
static struct list_head    _in_list;
static struct list_head    _out_list;

static audio_buffer_t   _audio_buffer[AUDIO_BUFFER_MAX_NUM];

//
// this doesn't have to be volatiles
// cuz optimization is already disabled by pragma
//
static volatile uint32_t _num_free = 0;
static volatile uint32_t _num_in = 0;
static volatile uint32_t _num_out = 0;

static volatile uint32_t _num_free_failed = 0;

//
// thought about volatile implementation of generic list library for some time.
// but my conclusion is this approach of disabling optimization on wrapper level
// is much better than a dirty ugly generic list implementation
// with full of volatiles.
// In the end, a generic library should be just a library.
//
#pragma GCC push_options
#pragma GCC optimize("O0")

static inline audio_buffer_t*
__audio_buffer_get(struct list_head* from)
{
  audio_buffer_t*   b;

  if(list_empty(from))
  {
    return NULL;
  }

  b = list_first_entry(from, audio_buffer_t, le);
  list_del_init(&b->le);

  return b;
}

static inline void
__audio_buffer_put(audio_buffer_t* b, struct list_head* to)
{
  list_add_tail(&b->le, to);
}

void
audio_buffer_init(void)
{
  INIT_LIST_HEAD((struct list_head*)&_free_list);
  INIT_LIST_HEAD((struct list_head*)&_in_list);
  INIT_LIST_HEAD((struct list_head*)&_out_list);

  for(int i = 0; i < AUDIO_BUFFER_MAX_NUM; i++)
  {
    INIT_LIST_HEAD(&_audio_buffer[i].le);
    audio_buffer_put_free(&_audio_buffer[i]);
  }
}

audio_buffer_t*
audio_buffer_get_free(void)
{
  audio_buffer_t* b= __audio_buffer_get(&_free_list);

  if(b)
  {
    _num_free--;
  }
  else
  {
    _num_free_failed++;
  }

  return b;
}

audio_buffer_t*
audio_buffer_get_in(void)
{
  audio_buffer_t* b= __audio_buffer_get(&_in_list);

  if(b) _num_in--;

  return b;
}

audio_buffer_t*
audio_buffer_get_out(void)
{
  audio_buffer_t* b= __audio_buffer_get(&_out_list);

  if(b) _num_out--;

  return b;
}

void
audio_buffer_put_free(audio_buffer_t* b)
{
  _num_free++;
  __audio_buffer_put(b, &_free_list);
}

void
audio_buffer_put_in(audio_buffer_t* b)
{
  _num_in++;
  __audio_buffer_put(b, &_in_list);
}

void
audio_buffer_put_out(audio_buffer_t* b)
{
  _num_out++;
  __audio_buffer_put(b, &_out_list);
}

void
audio_buffer_get_stat(audio_buffer_stat_t* stat)
{
  stat->num_free        = _num_free;
  stat->num_in          = _num_in;
  stat->num_out         = _num_out;
  stat->num_get_failed  = _num_free_failed;
}

#pragma GCC pop_options
