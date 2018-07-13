#ifndef __AUDIO_BUFFER_DEF_H__
#define __AUDIO_BUFFER_DEF_H__

#include "stm32f1xx_hal.h"
#include "generic_list.h"


#define AUDIO_BUFFER_SIZE               128
#define AUDIO_BUFFER_MAX_NUM            32

typedef struct
{
  struct list_head    le;
  volatile uint16_t   buffer[AUDIO_BUFFER_SIZE];
} audio_buffer_t;

extern void audio_buffer_init(void);
extern audio_buffer_t* audio_buffer_get_free(void);
extern audio_buffer_t* audio_buffer_get_in(void);
extern audio_buffer_t* audio_buffer_get_out(void);

extern void audio_buffer_put_free(audio_buffer_t* b);
extern void audio_buffer_put_in(audio_buffer_t* b);
extern void audio_buffer_put_out(audio_buffer_t* b);

#endif /* !__AUDIO_BUFFER_DEF_H__ */
