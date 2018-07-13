#ifndef __DAC_WRTITE_H__
#define __DAC_WRTITE_H__

extern void dac_write_init(void);
extern void dac_write_start(void);
extern void dac_write_copy(uint16_t* buf,  uint32_t len);

#endif /* !__DAC_WRTITE_H__ */
