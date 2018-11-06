#include "stm32f1xx_hal.h"
#include "usbd_cdc_if.h"

#include "app_common.h"

#include "shell_if_usb.h"
#include "shell.h"

#include "event_list.h"
#include "circ_buffer.h"
#include "event_list.h"
#include "event_dispatcher.h"

////////////////////////////////////////////////////////////////////////////////
//
// private variables
//
////////////////////////////////////////////////////////////////////////////////
static IRQn_Type              _irqn  = USB_LP_CAN1_RX0_IRQn;
static CircBuffer             _rx_cb;
static volatile uint8_t       _rx_buffer[CLI_RX_BUFFER_LENGTH];
static ShellIntf              _shell_usb_if;
static volatile bool          _initialized = false;

void
shell_if_usb_rx_notify(uint8_t* buf, uint32_t len)
{
  if(!_initialized) 
  {
    return;
  }

  //
  // runs in IRQ context
  //
  if(circ_buffer_enqueue(&_rx_cb, buf, len, true) == false)
  {
    // fucked up. overflow mostly.
    // do something here
  }

  event_set(1 << DISPATCH_EVENT_USB_CLI_RX);
}

////////////////////////////////////////////////////////////////////////////////
//
// shell callback
//
////////////////////////////////////////////////////////////////////////////////
static bool
shell_if_usb_get_rx_data(ShellIntf* intf, uint8_t* data)
{
  if(circ_buffer_dequeue(&_rx_cb, data, 1, false) == false)
  {
    return false;
  }
  return true;
}

static void
shell_if_usb_put_tx_data(ShellIntf* intf, uint8_t* data, uint16_t len)
{
  while(CDC_Transmit_FS(data, len) == USBD_BUSY)
  {
  }
}

static void
shell_if_usb_event_handler(uint32_t event)
{
  shell_handle_rx(&_shell_usb_if);
}

////////////////////////////////////////////////////////////////////////////////
//
// circular buffer callback
//
////////////////////////////////////////////////////////////////////////////////
static void
shell_if_usb_enter_critical(CircBuffer* cb)
{
  NVIC_DisableIRQ(_irqn);
}

static void
shell_if_usb_leave_critical(CircBuffer* cb)
{
  NVIC_EnableIRQ(_irqn);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
shell_if_usb_init(void)
{
  _shell_usb_if.cmd_buffer_ndx    = 0;
  _shell_usb_if.get_rx_data       = shell_if_usb_get_rx_data;
  _shell_usb_if.put_tx_data       = shell_if_usb_put_tx_data;

  INIT_LIST_HEAD(&_shell_usb_if.lh);

  circ_buffer_init(&_rx_cb, _rx_buffer, CLI_RX_BUFFER_LENGTH,
      shell_if_usb_enter_critical,
      shell_if_usb_leave_critical);

  shell_if_register(&_shell_usb_if);
  event_register_handler(shell_if_usb_event_handler, DISPATCH_EVENT_USB_CLI_RX);

  _initialized = true;
}
