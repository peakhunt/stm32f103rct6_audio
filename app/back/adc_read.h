#ifndef __ADC_READ_H__
#define __ADC_READ_H__

extern void adc_read_init(void);
extern void adc_read_start(void);
extern uint32_t adc_read_copy(uint16_t* buf);

#endif /* !__ADC_READ_H__ */
