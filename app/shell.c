#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "stm32f1xx_hal.h"

#include "app_common.h"
#include "shell.h"
#include "shell_if_usb.h"

#include "audio_buffer.h"
#include "dac_write.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////

#define SHELL_MAX_COLUMNS_PER_LINE      128
#define SHELL_COMMAND_MAX_ARGS          4

#define VERSION       "STM32F1 Shell V0.3a"

typedef void (*shell_command_handler)(ShellIntf* intf, int argc, const char** argv);

typedef struct
{
  const char*           command;
  const char*           description;
  shell_command_handler handler;
} ShellCommand;

////////////////////////////////////////////////////////////////////////////////
//
// private prototypes
//
////////////////////////////////////////////////////////////////////////////////
static void shell_command_help(ShellIntf* intf, int argc, const char** argv);
static void shell_command_version(ShellIntf* intf, int argc, const char** argv);
static void shell_command_uptime(ShellIntf* intf, int argc, const char** argv);
static void shell_command_bufs(ShellIntf* intf, int argc, const char** argv);
static void shell_command_dacs(ShellIntf* intf, int argc, const char** argv);
static void shell_command_bypass(ShellIntf* intf, int argc, const char** argv);

#ifdef FFT_TEST
static void shell_command_fft_test(ShellIntf* intf, int argc, const char** argv);
static void shell_command_fft_data(ShellIntf* intf, int argc, const char** argv);
#endif

////////////////////////////////////////////////////////////////////////////////
//
// private variables
//
////////////////////////////////////////////////////////////////////////////////
const uint8_t                 _welcome[] = "\r\n**** Welcome ****\r\n";
const uint8_t                 _prompt[]  = "\r\nSTM32F1> ";

static char                   _print_buffer[SHELL_MAX_COLUMNS_PER_LINE + 1];

static LIST_HEAD_DECL(_shell_intf_list);

static ShellCommand     _commands[] = 
{
  {
    "help",
    "show this command",
    shell_command_help,
  },
  {
    "version",
    "show version",
    shell_command_version,
  },
  {
    "uptime",
    "show system uptime",
    shell_command_uptime,
  },
  {
    "bufs",
    "show audio buffer stats",
    shell_command_bufs,
  },
  {
    "dacs",
    "show dac stats",
    shell_command_dacs,
  },
#ifdef FFT_TEST
  {
    "fft_test",
    "show fft test sample",
    shell_command_fft_test,
  },
  {
    "fft_data",
    "show fft test data",
    shell_command_fft_data,
  },
#endif
  {
    "bypass",
    "turn on/off bypass",
    shell_command_bypass,
  },
};

////////////////////////////////////////////////////////////////////////////////
//
// shell utilities
//
////////////////////////////////////////////////////////////////////////////////
static inline void
shell_prompt(ShellIntf* intf)
{
  intf->put_tx_data(intf, (uint8_t*)_prompt, sizeof(_prompt) -1);
}

////////////////////////////////////////////////////////////////////////////////
//
// shell command handlers
//
////////////////////////////////////////////////////////////////////////////////
static void
shell_command_help(ShellIntf* intf, int argc, const char** argv)
{
  size_t i;

  shell_printf(intf, "\r\n");

  for(i = 0; i < sizeof(_commands)/sizeof(ShellCommand); i++)
  {
    shell_printf(intf, "%-20s: ", _commands[i].command);
    shell_printf(intf, "%s\r\n", _commands[i].description);
  }
}

static void
shell_command_version(ShellIntf* intf, int argc, const char** argv)
{
  shell_printf(intf, "\r\n");
  shell_printf(intf, "%s\r\n", VERSION);
}

static void
shell_command_uptime(ShellIntf* intf, int argc, const char** argv)
{
  shell_printf(intf, "\r\n");
  shell_printf(intf, "System Uptime: %lu\r\n", __uptime);
}

static void
shell_command_bufs(ShellIntf* intf, int argc, const char** argv)
{
  audio_buffer_stat_t   stat;

  audio_buffer_get_stat(&stat);

  shell_printf(intf, "\r\n");
  shell_printf(intf, "free : %lu\r\n", stat.num_free);
  shell_printf(intf, "in   : %lu\r\n", stat.num_in);
  shell_printf(intf, "out  : %lu\r\n", stat.num_out);
  shell_printf(intf, "fail : %lu\r\n", stat.num_get_failed);
}

static void
shell_command_dacs(ShellIntf* intf, int argc, const char** argv)
{
  dac_write_stat_t    stat;

  dac_write_stat(&stat);

  shell_printf(intf, "\r\n");
  shell_printf(intf, "cont : %lu\r\n", stat.dac_cont);
  shell_printf(intf, "miss : %lu\r\n", stat.dac_cont_miss);
  shell_printf(intf, "irq  : %lu\r\n", stat.dac_irq);
}

#ifdef FFT_TEST
static void
shell_command_fft_test(ShellIntf* intf, int argc, const char** argv)
{
  extern float      _samples_f[2048];

  for(int i = 0; i <= 512; i++)
  {
    //shell_printf(intf, "%02d: %04x %04x\r\n", i, (uint16_t)_samples[i*2], (uint16_t)_samples[i*2+1]);
    shell_printf(intf, "%02d: %.4f %4f\r\n", i, _samples_f[i*2], _samples_f[i*2 + 1]);
  }
}

static void
shell_command_fft_data(ShellIntf* intf, int argc, const char** argv)
{
  extern float _test_in[1024];
  extern float _test_out[1024];

  for(int i = 0; i < 1024; i++)
  {
    if(fabs(_test_in[i] - _test_out[i]) > 0.0001f)
    {
      shell_printf(intf, "%02d: %.4f %.4f\r\n", i, _test_in[i], _test_out[i]);
    }
  }
}
#endif

static void
shell_command_bypass(ShellIntf* intf, int argc, const char** argv)
{
  extern bool _by_pass;

  _by_pass = !_by_pass;

  shell_printf(intf, "Turning %s bypass\r\n", _by_pass ? "ON" : "OFF");
}

////////////////////////////////////////////////////////////////////////////////
//
// shell core
//
////////////////////////////////////////////////////////////////////////////////
static void
shell_execute_command(ShellIntf* intf, char* cmd)
{
  static const char*    argv[SHELL_COMMAND_MAX_ARGS];
  int                   argc = 0;
  size_t                i;
  char                  *s, *t;

  while((s = strtok_r(argc  == 0 ? cmd : NULL, " \t", &t)) != NULL)
  {
    if(argc >= SHELL_COMMAND_MAX_ARGS)
    {
      shell_printf(intf, "\r\nError: too many arguments\r\n");
      return;
    }
    argv[argc++] = s;
  }

  if(argc == 0)
  {
    return;
  }

  for(i = 0; i < sizeof(_commands)/sizeof(ShellCommand); i++)
  {
    if(strcmp(_commands[i].command, argv[0]) == 0)
    {
      shell_printf(intf, "\r\nExecuting %s\r\n", argv[0]);
      _commands[i].handler(intf, argc, argv);
      return;
    }
  }
  shell_printf(intf, "%s", "\r\nUnknown Command: ");
  shell_printf(intf, "%s", argv[0]);
  shell_printf(intf, "%s", "\r\n");
}


void
shell_printf(ShellIntf* intf, const char* fmt, ...)
{
  va_list   args;
  int       len;

  va_start(args, fmt);
  len = vsnprintf(_print_buffer, SHELL_MAX_COLUMNS_PER_LINE, fmt, args);
  va_end(args);

  intf->put_tx_data(intf, (uint8_t*)_print_buffer, len);
}


////////////////////////////////////////////////////////////////////////////////
//
// public interface
//
////////////////////////////////////////////////////////////////////////////////
void
shell_init(void)
{
  shell_if_usb_init();
}

void
shell_start(void)
{
  ShellIntf* intf;

  list_for_each_entry(intf, &_shell_intf_list, lh)
  {
    intf->put_tx_data(intf, (uint8_t*)_welcome, sizeof(_welcome) -1);
    shell_prompt(intf);
  }
}


void
shell_if_register(ShellIntf* intf)
{
  list_add_tail(&intf->lh, &_shell_intf_list);
}

void
shell_handle_rx(ShellIntf* intf)
{
  uint8_t   b;

  while(1)
  {
    if(intf->get_rx_data(intf, &b) == false)
    {
      return;
    }

    if(b != '\r' && intf->cmd_buffer_ndx < SHELL_MAX_COMMAND_LEN)
    {
      if(b == '\b' || b == 0x7f)
      {
        if(intf->cmd_buffer_ndx > 0)
        {
          shell_printf(intf, "%c%c%c", b, 0x20, b);
          intf->cmd_buffer_ndx--;
        }
      }
      else
      {
        shell_printf(intf, "%c", b);
        intf->cmd_buffer[intf->cmd_buffer_ndx++] = b;
      }
    }
    else if(b == '\r')
    {
      intf->cmd_buffer[intf->cmd_buffer_ndx++] = '\0';

      shell_execute_command(intf, (char*)intf->cmd_buffer);

      intf->cmd_buffer_ndx = 0;
      shell_prompt(intf);
    }
  }
}

struct list_head*
shell_get_intf_list(void)
{
  return &_shell_intf_list;
}
