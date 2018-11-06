#ifndef __DAC_WRTITE_H__
#define __DAC_WRTITE_H__

#include "audio_buffer.h"

typedef struct
{
  uint32_t    dac_cont;
  uint32_t    dac_cont_miss;
  uint32_t    dac_irq;
} dac_write_stat_t;

extern void dac_write_init(void);
extern void dac_write_start(void);
extern void dac_write_put(audio_buffer_t* b);
extern void dac_write_stat(dac_write_stat_t* stat);

#endif /* !__DAC_WRTITE_H__ */
