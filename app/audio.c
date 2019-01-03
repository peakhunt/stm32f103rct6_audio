#include "audio.h"
#include "arm_math.h"
#include "arm_const_structs.h"

static arm_rfft_fast_instance_f32   _fft_f32;

static float32_t                    _fft_in_buf[AUDIO_BUFFER_SIZE];
static float32_t                    _fft_out_buf[AUDIO_BUFFER_SIZE];

void
audio_init(void)
{
  arm_rfft_fast_init_f32(&_fft_f32, AUDIO_BUFFER_SIZE);
}

void
audio_process(audio_buffer_t* b)
{
  //////////////////////////////////////////////////
  //
  // XXX
  //
  // sample size: 128
  // sampling frequency : 128 KHz
  //
  // then FFT frequency range
  // 128KHz/2 * linspace(0,1, 128)
  //
  // 64 / 128 = roughly 0.5KHz per each BIN
  //
  // so mostly indices between 0 and 15 are of primary
  // interest to me.
  //
  //////////////////////////////////////////////////

  for(int i = 0; i < AUDIO_BUFFER_SIZE; i++)
  {
    _fft_in_buf[i] = b->buffer[i];
  }

  // time domain to frequency domain
  arm_rfft_fast_f32(&_fft_f32, _fft_in_buf, _fft_out_buf, 0);

  //
  // FIXME
  // add DSP processing
  //

  // frequency domain to time domain
  arm_rfft_fast_f32(&_fft_f32, _fft_out_buf, _fft_in_buf, 1);

  for(int i = 0; i < AUDIO_BUFFER_SIZE; i++)
  {
    b->buffer[i] = (uint16_t)(_fft_in_buf[i]);
  }
}
