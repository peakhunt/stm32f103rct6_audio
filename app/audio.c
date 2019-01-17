#include "audio.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_LEN               AUDIO_BUFFER_SIZE
#define REAL_PART(x)          (x * 2 + 0)
#define IMAG_PART(x)          (x * 2 + 1)

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
static arm_rfft_instance_q15    _fwd_fft;
static arm_rfft_instance_q15    _inv_fft;

static q15_t                    _input_buffer[FFT_LEN];

static q15_t                    _input_offset_neg_0p5 = 0xc000;
static q15_t                    _input_scale_0p250062 = 0x2002;
static q15_t                    _output_scale_0p124969 = 0xfff;
static q15_t                    _output_offset_0p124969_div_2 = 0x80b;

static q15_t                    _magnitudes[FFT_LEN * 2];
static q15_t                    _output_buffer[FFT_LEN * 2];

////////////////////////////////////////////////////////////////////////////////
//
// private DSP kernels
//
////////////////////////////////////////////////////////////////////////////////
static void
audio_process_bypass(q15_t* mag, int len)
{
  len++;
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
audio_init(void)
{
  if(arm_rfft_init_q15(&_fwd_fft, FFT_LEN, 0, 1) != ARM_MATH_SUCCESS)
  {
    while(1)
      ;
  }

  if(arm_rfft_init_q15(&_inv_fft, FFT_LEN, 1, 1) != ARM_MATH_SUCCESS)
  {
    while(1)
      ;
  }
}

void
audio_process(audio_buffer_t* b)
{
#if 0
  for(int i = 0; i < FFT_LEN; i++)
  {
    b->buffer[i] <<= 3;
  }
#endif
  //
  // ADC input is 12 bit in the range of 0 - 4095
  //
  // in Q15, 0 is 0 and 4095 is 0.124969
  //
  // for maximum accuracy, we wanna scale (0~0.124969) in Q15
  // to (-1 ~ 1) in Q15
  //

  // scale 0 ~ 0.124969 to 0 ~ 1
  // scale = SF * 2^shift
  // 1/0.124969 = 8.001984492154054
  //            = 0.250062015379814 * 32
  //            = 0.250062015379814 * 2^5
  arm_scale_q15((q15_t*)b->buffer, _input_scale_0p250062, 5, _input_buffer, FFT_LEN);
  // scale 0 ~ 1 to -0.5 to 0.5
  arm_offset_q15(_input_buffer, _input_offset_neg_0p5, _input_buffer, FFT_LEN);
  // scale -0.5 to 0.5 to -1.0 to 1.0
  arm_shift_q15(_input_buffer, 1, _input_buffer, FFT_LEN);    // primary accuracy loss here suspected.

  arm_rfft_q15(&_fwd_fft, _input_buffer, _magnitudes);
  //
  // XXX what a confusing doc!
  // 1/(fftlen/2) scale doesn't apply here! why???
  //arm_shift_q15(_magnitudes, 6, _magnitudes, FFT_LEN * 2);

  audio_process_bypass(_magnitudes, FFT_LEN);

  arm_rfft_q15(&_inv_fft, _magnitudes, _output_buffer);
  //
  // ifft output is so small, and 1/(fftlen/2) scale mentioned in the doc
  // makes sense but too much loss in accuracy! not sure if this is expected with q15 FFT
  //
  arm_shift_q15(_output_buffer, 6, _output_buffer, FFT_LEN);    // primary accuracy loss here suspected.

  // scale -1 ~ 1 to -0.124969/2 ~ 0.124969/2
  arm_scale_q15(_output_buffer, _output_scale_0p124969, -1, _output_buffer, FFT_LEN);
  // scale -0.124969/2 ~ 0.124969/2 to 0 ~ 0.124969
  arm_offset_q15(_output_buffer, _output_offset_0p124969_div_2, (q15_t*)b->buffer, FFT_LEN);
}
