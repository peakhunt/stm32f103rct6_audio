#include <string.h>
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

#include "adc_read.h"
#include "buffer_list.h"

#define ADC_BUFFER_LENGTH           256
#define ADC_BUFFER_HALF_LENGTH      (ADC_BUFFER_LENGTH/2)

#define ADC_NUM_BUFFERS             16

////////////////////////////////////////////////////////////////////////////////
//
// ADC FIFO
//
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  buffer_head_t       head;
  volatile uint16_t   buffer[ADC_BUFFER_HALF_LENGTH];
} adc_in_buffer_t;

static adc_in_buffer_t  _buffers[ADC_NUM_BUFFERS];
static buffer_list_t    _buffer_list;

////////////////////////////////////////////////////////////////////////////////
//
// ADC DMA buffer
//
////////////////////////////////////////////////////////////////////////////////
static uint16_t _adc_buffer[ADC_BUFFER_LENGTH];
static uint8_t _test_cnt = 0;


////////////////////////////////////////////////////////////////////////////////
//
// ADC fifo management
//
////////////////////////////////////////////////////////////////////////////////
static void
adc_buffer_init(void)
{
  buffer_list_init(&_buffer_list);

  for(int i = 0; i < ADC_NUM_BUFFERS; i++)
  {
    buffer_list_head_init(&_buffers[i].head);
    buffer_list_add_free(&_buffer_list, &_buffers[i].head);
  }
}

static void
adc_buffer_enqueue(uint16_t* buf)
{
  buffer_head_t*    bh;
  adc_in_buffer_t*  b;

  if(buffer_list_num_free(&_buffer_list) <= 0)
  {
    // FIXME
    // buffer overflow stats
    return;
  }

  bh = buffer_list_get_free(&_buffer_list);
  b = container_of(bh, adc_in_buffer_t, head);

  memcpy((void*)b->buffer, buf, sizeof(uint16_t)*ADC_BUFFER_HALF_LENGTH);

  buffer_list_add_used(&_buffer_list, bh);
}

static uint32_t
adc_buffer_dequeue(uint16_t* buf)
{
  uint32_t ret = 0;

  if(buffer_list_num_used(&_buffer_list) != 0)
  {
    buffer_head_t*    bh;
    adc_in_buffer_t*  b;

    bh = buffer_list_get_used(&_buffer_list);
    b = container_of(bh, adc_in_buffer_t, head);

    memcpy(buf, (void*)b->buffer, sizeof(uint16_t)*ADC_BUFFER_HALF_LENGTH);

    buffer_list_add_free(&_buffer_list, bh);
    ret = ADC_BUFFER_HALF_LENGTH;
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// ADC DMA IRQ Callbacks
//
////////////////////////////////////////////////////////////////////////////////
void
HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  adc_buffer_enqueue(&_adc_buffer[0]);
}

void
HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  // copy
  _test_cnt++;

  if(_test_cnt >= 50)
  {
    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    _test_cnt = 0;
  }
  adc_buffer_enqueue(&_adc_buffer[ADC_BUFFER_HALF_LENGTH]);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces for app
//
////////////////////////////////////////////////////////////////////////////////
void
adc_read_init(void)
{
  adc_buffer_init();
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (72000000 / 128000) -1);
}

void
adc_read_start(void)
{
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)_adc_buffer, ADC_BUFFER_LENGTH);
}

uint32_t
adc_read_copy(uint16_t* buf)
{
  uint32_t    ret;

  NVIC_DisableIRQ(DMA1_Channel1_IRQn);

  ret = adc_buffer_dequeue(buf);

  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  return ret;
}
