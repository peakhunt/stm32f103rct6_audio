#include <string.h>
#include "stm32f1xx_hal.h"
#include "dac.h"
#include "tim.h"
#include "gpio.h"

#include "dac_write.h"
#include "buffer_list.h"

#define DAC_BUFFER_LENGTH       128
#define DAC_NUM_BUFFERS             16

////////////////////////////////////////////////////////////////////////////////
//
// DAC FIFO
//
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  buffer_head_t       head;
  volatile uint16_t   buffer[DAC_BUFFER_LENGTH];
} dac_out_buffer_t;

static dac_out_buffer_t   _buffers[DAC_NUM_BUFFERS];
static buffer_list_t      _buffer_list;

////////////////////////////////////////////////////////////////////////////////
//
// DAC DMA buffer
//
////////////////////////////////////////////////////////////////////////////////
static volatile uint16_t _dac_buffer[DAC_BUFFER_LENGTH];
static uint8_t  _test_cnt = 0;
static volatile uint8_t  _dma_in_progress = 0;

////////////////////////////////////////////////////////////////////////////////
//
// DAC fifo management
//
////////////////////////////////////////////////////////////////////////////////
static void
dac_buffer_init(void)
{
  buffer_list_init(&_buffer_list);

  for(int i = 0; i < DAC_NUM_BUFFERS; i++)
  {
    buffer_list_head_init(&_buffers[i].head);
    buffer_list_add_free(&_buffer_list, &_buffers[i].head);
  }
}

static void
dac_buffer_enqueue(uint16_t* buf)
{
  buffer_head_t*    bh;
  dac_out_buffer_t*  b;

  if(buffer_list_num_free(&_buffer_list) <= 0)
  {
    // FIXME
    // buffer overflow stats
    return;
  }

  bh = buffer_list_get_free(&_buffer_list);
  b = container_of(bh, dac_out_buffer_t, head);

  memcpy((void*)b->buffer, buf, sizeof(uint16_t)*DAC_BUFFER_LENGTH);

  buffer_list_add_used(&_buffer_list, bh);
}

static uint32_t
dac_buffer_dequeue(uint16_t* buf)
{
  uint32_t ret = 0;

  if(buffer_list_num_used(&_buffer_list) != 0)
  {
    buffer_head_t*    bh;
    dac_out_buffer_t*  b;

    bh = buffer_list_get_used(&_buffer_list);
    b = container_of(bh, dac_out_buffer_t, head);

    memcpy(buf, (void*)b->buffer, sizeof(uint16_t)*DAC_BUFFER_LENGTH);

    buffer_list_add_free(&_buffer_list, bh);
    ret = DAC_BUFFER_LENGTH;
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// DAC DMA IRQ Callbacks
//
////////////////////////////////////////////////////////////////////////////////
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

  if(_test_cnt >= 50)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
    _test_cnt = 0;
  }

  if(dac_buffer_dequeue((uint16_t*)_dac_buffer))
  {
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)_dac_buffer, DAC_BUFFER_LENGTH, DAC_ALIGN_12B_L);
  }
  else
  {
    _dma_in_progress = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// DAC public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
dac_write_init(void)
{
  dac_buffer_init();
}

void
dac_write_start(void)
{
  HAL_TIM_Base_Start(&htim2);
  // HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)_dac_buffer, DAC_BUFFER_LENGTH, DAC_ALIGN_12B_L);
}

void
dac_write_copy(uint16_t* buf,  uint32_t len)
{
  NVIC_DisableIRQ(DMA2_Channel3_IRQn);
  if(_dma_in_progress)
  {
    dac_buffer_enqueue(buf);
  }
  else
  {
    memcpy((void*)_dac_buffer, buf, sizeof(uint16_t)*DAC_BUFFER_LENGTH);
    _dma_in_progress = 1;
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)_dac_buffer, DAC_BUFFER_LENGTH, DAC_ALIGN_12B_L);
  }
  NVIC_EnableIRQ(DMA2_Channel3_IRQn);
}
