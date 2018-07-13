#ifndef __ADC_READ_H__
#define __ADC_READ_H__

#include "audio_buffer.h"

extern void adc_read_init(void);
extern void adc_read_start(void);
extern audio_buffer_t* adc_read_in(void);

#endif /* !__ADC_READ_H__ */
