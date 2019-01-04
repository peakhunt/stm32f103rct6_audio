#include "audio.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_LEN                     AUDIO_BUFFER_SIZE

////////////////////////////////////////////////////////////////////////////////
//
// module prototypes
//
////////////////////////////////////////////////////////////////////////////////
static void audio_process_bypass(q15_t* mag, int len);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static arm_rfft_instance_q15        _fft;

static q15_t                        _magnitudes[FFT_LEN];
static q15_t                        _samples[FFT_LEN];

////////////////////////////////////////////////////////////////////////////////
//
// private DSP kernels
//
////////////////////////////////////////////////////////////////////////////////
static void
audio_process_bypass(q15_t* mag, int len)
{
  // a simple bypass DSP kernel
  // don't do anything
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
audio_init(void)
{
  // nothing to do
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

  // forward FFT
  arm_rfft_init_q15(&_fft, FFT_LEN, 0, 1);
  arm_rfft_q15(&_fft, (q15_t*)b->buffer, _magnitudes);

  audio_process_bypass(_magnitudes, FFT_LEN);

  // inverse FFT
  arm_rfft_init_q15(&_fft, FFT_LEN, 1, 1);
  arm_rfft_q15(&_fft,  _magnitudes, _samples);

  //
  // XXX
  // convert _samples in q15 to uint16_t dac output
  //
  // No we don't have to do anything as long as we manipulate _magnitudes
  // in q15 arithmetic.
  //
}
