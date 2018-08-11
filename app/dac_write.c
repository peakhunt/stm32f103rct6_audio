#include <string.h>
#include "stm32f1xx_hal.h"
#include "dac.h"
#include "tim.h"
#include "gpio.h"

#include "dac_write.h"
#include "audio_buffer.h"

#define DAC_BUFFER_LENGTH       AUDIO_BUFFER_SIZE

static volatile uint16_t _dac_buffer[DAC_BUFFER_LENGTH];
static uint8_t  _test_cnt = 0;
static volatile audio_buffer_t*   _current_buffer = NULL;

static uint32_t   _num_dac_cont = 0;
static uint32_t   _num_dac_cont_miss = 0;
static uint32_t   _num_dac_irq = 0;

////////////////////////////////////////////////////////////////////////////////
//
// DAC DMA IRQ Callbacks
//
////////////////////////////////////////////////////////////////////////////////
static void
dac_start_dma(uint8_t from_isr)
{
  audio_buffer_t* b;

  b = audio_buffer_get_out();
  if(b == NULL)
  {
    if(from_isr)
    {
      _num_dac_cont_miss++;
    }
    return;
  }

  if(from_isr)
  {
    _num_dac_cont++;
  }

  _current_buffer = b;

  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)b->buffer, AUDIO_BUFFER_SIZE, DAC_ALIGN_12B_R);
}

#if 0
void
HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* arg_hdac)
{
}
#endif

void
HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* arg_hdac)
{
  _test_cnt++;
  _num_dac_irq++;

  if(_test_cnt >= 50)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    _test_cnt = 0;
  }

  audio_buffer_put_free((audio_buffer_t*)_current_buffer);
  _current_buffer = NULL;

  dac_start_dma(1);
}

////////////////////////////////////////////////////////////////////////////////
//
// DAC public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
dac_write_init(void)
{
}

void
dac_write_start(void)
{
  HAL_TIM_Base_Start(&htim2);
}

void
dac_write_put(audio_buffer_t* b)
{
  NVIC_DisableIRQ(DMA2_Channel3_IRQn);
  __DSB();
  __ISB();

  audio_buffer_put_out(b);
  if(_current_buffer == NULL)
  {
    dac_start_dma(0);
  }

  NVIC_EnableIRQ(DMA2_Channel3_IRQn);
}
