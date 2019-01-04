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

#ifdef FFT_TEST
float32_t               _test_in[1024];
float32_t               _test_out[1024];
float32_t               _samples_f[2048];
float32_t               _samples_copied[2048];
#endif

////////////////////////////////////////////////////////////////////////////////
//
// private DSP kernels
//
////////////////////////////////////////////////////////////////////////////////
static void
audio_process_bypass(q15_t* mag, int len)
{
  //
  // XXX
  // this is just a sample to remember
  // how to handle FFT data
  //

  q15_t   real, imag;
  // remember!!!
  //
  // to calculate magnitude,
  // for real part, 
  //                        real[i] / (N/2)
  //                        real[i] / N for i = 0 and N/2
  // for imaginary part,    -imaginary[i] / (N/2)
  //
  //

  for(int i = 0; i <= len / 2; i++)
  {
    real = mag[i * 1 + 0];       // cos part
    imag = mag[i * 1 + 1];       // sin part

    (void)real;
    (void)imag;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
audio_init(void)
{
#ifdef FFT_TEST
  float32_t   s, c;
  float32_t   t;

  for(int i = 0; i < 1024; i++)
  {
    t = (3.14f * 6.0f/ 1024.0f) * i;

    s = arm_sin_f32(2 * t);
    c = arm_cos_f32(4 * t);

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
#endif
}

void
audio_process(audio_buffer_t* b)
{
  //
  // XXX
  // this copy after forward/inverse FFT looks quite a waste of resource.
  //

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
  // XXX : how to get rid of this copy
  //
  // prepare forward CFFT input
  //
  for(int i = 0; i < FFT_LEN ; i++)
  {
    _samples[i * 2 + 0] = b->buffer[i];
    _samples[i * 2 + 1] = 0;    // Q15 0
  }

  // forward FFT
  // input is [real | imaginary] format
  //
  arm_cfft_q15(&arm_cfft_sR_q15_len128, _samples, 0, 1);
  //
  // output is magnitude of [real | imaginary] format
  //
  // to calculate magnitude,
  // for real part, 
  //                        real[i] / (N/2)
  //                        real[i] / N for i = 0 and N/2
  // for imaginary part,    -imaginary[i] / (N/2)
  //
  // i = from 0 to N/2 inclusive, that is, N/2 + 1 magnitudes for each part.
  //
  audio_process_bypass(_samples, FFT_LEN);

  //
  // input is magnitude of [real | imaginary]
  //
  arm_cfft_q15(&arm_cfft_sR_q15_len128, _samples, 1, 1);

  //
  // output is FFT_LEN time domain signals of [real | imaginary] 
  //
  // XXX : how to get rid of this copy
  //
  for(int i = 0; i < FFT_LEN ; i++)
  {
    b->buffer[i] = (uint16_t)_samples[i * 2 + 0];
  }
}
