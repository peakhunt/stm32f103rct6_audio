#ifndef __AUDIO_DEF_H__
#define __AUDIO_DEF_H__

#include "audio_buffer.h"

extern void audio_init(void);
extern void audio_process(audio_buffer_t* b);

#endif /* !__AUDIO_DEF_H__ */
