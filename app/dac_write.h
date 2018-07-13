#ifndef __DAC_WRTITE_H__
#define __DAC_WRTITE_H__

#include "audio_buffer.h"

extern void dac_write_init(void);
extern void dac_write_start(void);
extern void dac_write_put(audio_buffer_t* b);

#endif /* !__DAC_WRTITE_H__ */
