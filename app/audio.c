#include "audio.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_LEN                     AUDIO_BUFFER_SIZE

static float32_t                    _samples[FFT_LEN * 2];
static float32_t                    _magnitudes[FFT_LEN];

void
audio_init(void)
{
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
  // is it normal for DSP to be this much slow in CM3?
  //
  //////////////////////////////////////////////////

  for(int i = 0; i < FFT_LEN * 2; i += 2)
  {
    _samples[i] = b->buffer[i/2];     // real part
    _samples[i + 1] = 0;              // imaginary part
  }

  // time domain to frequency domain
  arm_cfft_f32(&arm_cfft_sR_f32_len128, _samples, 0, 1);
  arm_cmplx_mag_f32(_samples, _magnitudes, FFT_LEN);

  //
  // FIXME
  // add DSP processing
  //

  // frequency domain to time domain
  arm_cmplx_mag_f32(_samples, _magnitudes, FFT_LEN);
  arm_cfft_f32(&arm_cfft_sR_f32_len128, _samples, 1, 1);

  for(int i = 0; i < FFT_LEN * 2; i += 2)
  {
    b->buffer[i/2] = (uint16_t)(_samples[i]);
    // XXX ignore imaginary part
  }
}
