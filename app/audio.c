#include "audio.h"
#include "arm_math.h"

static arm_fir_instance_f32   _fir_filter;
static float32_t              _fir_coeffs[1] = { 0.00f };
static float32_t              _fir_states[1] = { 0.00f };

void
audio_init(void)
{
  arm_fir_f32(
      &_fir_filter,
      _fir_coeffs,
      _fir_states,
      64);
}

void
audio_process(audio_buffer_t* b)
{
  (void)_fir_filter;
}
