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
q15_t                               _samples[FFT_LEN*2];

float32_t               _test_in[1024];
float32_t               _test_out[1024];
float32_t               _samples_f[2048];
float32_t               _samples_copied[2048];

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
  float32_t   s, c;
  float32_t   t;

  for(int i = 0; i < 1024; i++)
  {
    t = (3.14f * 6.0f/ 1024.0f) * i;

    s = arm_sin_f32(t);
    c = arm_cos_f32(t);

    _test_in[i] = 3.0f * s + 1.5f * c;
  }

  for(int i = 0; i < 1024; i++)
  {
    _samples_f[i * 2 + 0] = _test_in[i];
    _samples_f[i * 2 + 1] = 0.0f;
  }

  // forward
  arm_cfft_f32(&arm_cfft_sR_f32_len1024, _samples_f, 0, 1);

  //
  // copy magnitude data to save and later recover
  //
  for(int i = 0; i < 2048; i++)
  {
    _samples_copied[i] = _samples_f[i];
  }


  // inverse
  arm_cfft_f32(&arm_cfft_sR_f32_len1024, _samples_copied, 1, 1);

  for(int i = 0; i < 1024; i++)
  {
    _test_out[i] = _samples_copied[i * 2 + 0];
  }
}

void
audio_process(audio_buffer_t* b)
{
  //////////////////////////////////////////////////
  //
  // sample size: 128
  // sampling frequency : 128 KHz
  //
  // then FFT frequency range
  // 128KHz/2 * linspace(0,1, 128)
  //
  // 64 / 128 = roughly 0.5KHz per each BIN
  //
  //////////////////////////////////////////////////

  //
  // prepare forward CFFT input
  //
  for(int i = 0; i < FFT_LEN ; i++)
  {
    _samples[i * 2 + 0] = b->buffer[i];
    _samples[i * 2 + 1] = 0;    // Q15 0
  }

  // forward FFT
  arm_cfft_q15(&arm_cfft_sR_q15_len128, _samples, 0, 1);

  audio_process_bypass(_samples, FFT_LEN);

#if 0
  // inverse FFT
  arm_cfft_q15(&arm_cfft_sR_q15_len128, _samples, 1, 1);

  //
  // prepare inverse CFFT output
  //
  for(int i = 0; i < FFT_LEN ; i++)
  {
    b->buffer[i] = (uint16_t)_samples[i * 2 + 0];
  }
#endif
}
