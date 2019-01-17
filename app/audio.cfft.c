#include "audio.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_LEN                       AUDIO_BUFFER_SIZE
#if 0
#define FFT_CONTEXT                   (&(arm_cfft_sR_q31_len128))
#define FFT_FUNC                      arm_cfft_q31
typedef q31_t                         fft_data_t;
#else
#define FFT_CONTEXT                   (&(arm_cfft_sR_q15_len128))
#define FFT_FUNC                      arm_cfft_q15
typedef q15_t                         fft_data_t;
#endif

#define REAL_PART(x)          (x * 2 + 0)
#define IMAG_PART(x)          (x * 2 + 1)

////////////////////////////////////////////////////////////////////////////////
//
// module prototypes
//
////////////////////////////////////////////////////////////////////////////////
static void audio_process_bypass(fft_data_t* mag, int len);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static fft_data_t                     _samples[FFT_LEN * 2];
static fft_data_t                     _input_scale_0p25[FFT_LEN];
static fft_data_t                     _input_buffer[FFT_LEN];

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
audio_process_bypass(fft_data_t* mag, int len)
{
#if 0
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
    real = mag[i * 2 + 0];       // cos part
    imag = mag[i * 2 + 1];       // sin part

    (void)real;
    (void)imag;
  }
#endif
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

  for(int i = 0; i < FFT_LEN; i++)
  {
    _input_scale_0p25[i] = 0x2000;
  }
}

void
audio_process(audio_buffer_t* b)
{
#if 0
  arm_rfft_init_q15(&_q15_fft_128, FFT_LEN, 0, 1);
  arm_rfft_q15(&_q15_fft_128, (q15_t*)b->buffer, _temp_buf);
  arm_shift_q15(_temp_buf, 12, _magnitudes, FFT_LEN * 2);

  audio_process_bypass(_magnitudes, FFT_LEN);

  arm_rfft_init_q15(&_q15_fft_128, FFT_LEN, 1, 1);
  arm_rfft_q15(&_q15_fft_128, _magnitudes, _temp_buf);
  arm_shift_q15(_temp_buf, 12, (q15_t*)b->buffer, FFT_LEN);
#endif
  //
  // let's do some int16_t arithmetic
  //
  // to achieve max accuracy with q15_t and 12bit unsigned ADC
  // we should convert
  //
  // q15 = ((int16_t)(adc_raw - 2048) << 3)
  // adc_raw = (uint16_t)((q15 >> 3) + 2048)
  //

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

  // scale input in Q15 so that it is between 0 ~ 0.5. mul by 4
  arm_shift_q15((q15_t*)b->buffer, 2, _input_buffer, FFT_LEN);

  // scale input in Q15 so that is is between -0.25 ~ 0.25. sub by 0.25
  arm_sub_q15(_input_buffer, _input_scale_0p25, _input_buffer, FFT_LEN);

  // scale input in Q15 so that it is between -1 ~ 1. mul by 4
  arm_shift_q15(_input_buffer, 2, _input_buffer, FFT_LEN);

  // prepare compex buffer
  for(int i = 0; i < FFT_LEN ; i++)
  {
    _samples[REAL_PART(i)] = _input_buffer[i];
    _samples[IMAG_PART(i)] = 0;    // Q15 0
  }

  // forward FFT
  // input is [real | imaginary] format
  //
  FFT_FUNC(FFT_CONTEXT, _samples, 0, 1);

  // arm_cmplx_mag_q15(_samples, _samples, FFT_LEN);
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
  FFT_FUNC(FFT_CONTEXT, _samples, 1, 1);

  //
  // output is FFT_LEN time domain signals of [real | imaginary] 
  //
  // XXX : how to get rid of this copy
  //
  for(int i = 0; i < FFT_LEN ; i++)
  {
    b->buffer[i] = (uint16_t)(_samples[REAL_PART(i)]);
  }
}
