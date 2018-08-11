#include <string.h>
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

#include "adc_read.h"
#include "audio_buffer.h"

#define ADC_BUFFER_HALF_LENGTH      AUDIO_BUFFER_SIZE
#define ADC_BUFFER_LENGTH           (2*ADC_BUFFER_HALF_LENGTH)

////////////////////////////////////////////////////////////////////////////////
//
// ADC DMA buffer
//
////////////////////////////////////////////////////////////////////////////////
static uint16_t _adc_buffer[ADC_BUFFER_LENGTH];
static uint8_t _test_cnt = 0;

static uint32_t _num_adc_irq = 0;

////////////////////////////////////////////////////////////////////////////////
//
// ADC DMA IRQ Callbacks
//
////////////////////////////////////////////////////////////////////////////////
static inline void
copy_adc_data_and_put(uint16_t* buf)
{
  audio_buffer_t* b;

  b = audio_buffer_get_free();
  if(b == NULL)
  {
    return;
  }

  memcpy((uint16_t*)b->buffer, buf, sizeof(uint16_t) * AUDIO_BUFFER_SIZE);
  audio_buffer_put_in(b);
}

void
HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  copy_adc_data_and_put(&_adc_buffer[0]);
  _num_adc_irq++;
}

void
HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  _test_cnt++;

  if(_test_cnt >= 50)
  {
    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    _test_cnt = 0;
  }
  copy_adc_data_and_put(&_adc_buffer[ADC_BUFFER_HALF_LENGTH]);
  _num_adc_irq++;
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces for app
//
////////////////////////////////////////////////////////////////////////////////
void
adc_read_init(void)
{
  if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    while(1)
    {
    }
  }

  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (72000000 / 128000) -1);
}

void
adc_read_start(void)
{
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)_adc_buffer, ADC_BUFFER_LENGTH);
}

audio_buffer_t*
adc_read_in(void)
{
  audio_buffer_t* b;

  NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  __DSB();
  __ISB();

  b = audio_buffer_get_in();

  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  return b;
}
