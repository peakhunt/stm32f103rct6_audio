//
// read this
// https://www.kernel.org/doc/html/latest/process/volatile-considered-harmful.html
//
#include "audio_buffer.h"

static volatile struct volatile_list_head    _free_list;
static volatile struct volatile_list_head    _in_list;
static volatile struct volatile_list_head    _out_list;

static audio_buffer_t   _audio_buffer[AUDIO_BUFFER_MAX_NUM];

static volatile uint32_t _num_free = 0;
static volatile uint32_t _num_in = 0;
static volatile uint32_t _num_out = 0;

static volatile uint32_t _num_free_failed = 0;

static inline audio_buffer_t*
__audio_buffer_get(volatile struct volatile_list_head* from)
{
  audio_buffer_t*   b;

  if(volatile_list_empty(from))
  {
    return NULL;
  }

  b = list_first_entry(from, audio_buffer_t, le);
  volatile_list_del_init(&b->le);

  return b;
}

static inline void
__audio_buffer_put(audio_buffer_t* b, volatile struct volatile_list_head* to)
{
  volatile_list_add_tail(&b->le, to);
}

void
audio_buffer_init(void)
{
  VOLATILE_INIT_LIST_HEAD(&_free_list);
  VOLATILE_INIT_LIST_HEAD(&_in_list);
  VOLATILE_INIT_LIST_HEAD(&_out_list);

  for(int i = 0; i < AUDIO_BUFFER_MAX_NUM; i++)
  {
    VOLATILE_INIT_LIST_HEAD(&_audio_buffer[i].le);
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
