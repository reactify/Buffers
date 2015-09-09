/**
 * Copyright (c) 2014,2015 Enzien Audio, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, and/or
 * sublicense copies of the Software, strictly on a non-commercial basis,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _HEAVY_SIGNAL_PHASOR_H_
#define _HEAVY_SIGNAL_PHASOR_H_

#include "HvBase.h"

// The reciprocal of the maximum value represented by a 32-bit floating point
// number’s mantissa - used to scale the wrap around point in the phasor
#define __HV_PHASOR_SCALE 0.0000001192093f // ((2^23)-1)^-1

typedef struct SignalPhasor {
  union {
    float f2sc; // float to step conversion (used for __phasor~f)
    hv_int32_t s; // step value (used for __phasor_k~f)
  } step;
#if HV_SIMD_AVX || HV_SIMD_SSE
  __m128i phase; // current phase
  __m128i inc; // phase increment
#elif HV_SIMD_NEON
  uint32x4_t phase;
  int32x4_t inc;
#else // HV_SIMD_NONE
  hv_uint32_t phase;
  hv_int32_t inc;
#endif
} SignalPhasor;

hv_size_t sPhasor_init(SignalPhasor *o, double samplerate);

hv_size_t sPhasor_k_init(SignalPhasor *o, float frequency, double samplerate);

void sPhasor_k_onMessage(HvBase *_c, SignalPhasor *o, int letIn, const HvMessage *m);

void sPhasor_onMessage(HvBase *_c, SignalPhasor *o, int letIn, const HvMessage *m);

static inline void __hv_phasor_f(SignalPhasor *o, hv_bInf_t bIn, hv_bOutf_t bOut) {
#if HV_SIMD_AVX
  static const __m256 c = (__m256) {
      __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE,
      __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE};

  __m256i s256 = _mm256_cvtps_epi32(_mm256_mul_ps(bIn, _mm256_set1_ps(o->step.f2sc))); // convert frequency to step

  // add incremental steps to phase
  __m128i s = _mm256_extractf128_si256(s256, 0); // lower vector
  s = _mm_add_epi32(s, _mm_slli_si128(s, 4));
  s = _mm_add_epi32(s, _mm_slli_si128(s, 8));
  __m128i p = _mm_add_epi32(o->phase, s);

  __m128i q = _mm_shuffle_epi32(p, _MM_SHUFFLE(3,3,3,3));
  s = _mm256_extractf128_si256(s256, 1); // upper vector
  s = _mm_add_epi32(s, _mm_slli_si128(s, 4));
  s = _mm_add_epi32(s, _mm_slli_si128(s, 8));
  q = _mm_add_epi32(q, s);

  o->phase = _mm_shuffle_epi32(q, _MM_SHUFFLE(3,3,3,3));

  p = _mm_srli_epi32(p, 9);
  q = _mm_srli_epi32(q, 9);
  *bOut = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_insertf128_si256(_mm256_castsi128_si256(p), q, 1)), c);
#elif HV_SIMD_SSE
  static const __m128 c = (__m128) {__HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE};
  __m128i p = _mm_cvtps_epi32(_mm_mul_ps(bIn, _mm_set1_ps(o->step.f2sc))); // convert frequency to step
  p = _mm_add_epi32(p, _mm_slli_si128(p, 4)); // add incremental steps to phase (prefix sum)
  p = _mm_add_epi32(p, _mm_slli_si128(p, 8)); // http://stackoverflow.com/questions/10587598/simd-prefix-sum-on-intel-cpu?rq=1
  p = _mm_add_epi32(o->phase, p);
  *bOut = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srli_epi32(p, 9)), c); // convert integer phase to float in range [0,1]
  o->phase = _mm_shuffle_epi32(p, _MM_SHUFFLE(3,3,3,3));
#elif HV_SIMD_NEON
  int32x4_t p = vcvtq_s32_f32(vmulq_n_f32(bIn, o->step.f2sc));
  p = vaddq_s32(p, vextq_s32(vdupq_n_s32(0), p, 3)); // http://stackoverflow.com/questions/11259596/arm-neon-intrinsics-rotation
  p = vaddq_s32(p, vextq_s32(vdupq_n_s32(0), p, 2));
  uint32x4_t pp = vaddq_u32(o->phase, vreinterpretq_u32_s32(p));
  *bOut = vmulq_n_f32(vcvtq_f32_u32(vshrq_n_u32(pp, 9)), __HV_PHASOR_SCALE);
  o->phase = vdupq_n_u32(pp[3]);
#else // HV_SIMD_NONE
  *bOut = ((float) (o->phase >> 9)) * __HV_PHASOR_SCALE;
  o->phase += ((int) (bIn * o->step.f2sc));
#endif
}

static inline void __hv_phasor_k_f(SignalPhasor *o, hv_bOutf_t bOut) {
#if HV_SIMD_AVX
  static const __m256 c = (__m256) {
      __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE,
      __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE};
  __m128i inc = o->inc;
  __m128i phase = o->phase;
  __m128i p = _mm_srli_epi32(phase, 9);
  phase = _mm_add_epi32(phase, inc);
  __m128i q = _mm_srli_epi32(phase, 9);
  o->phase = _mm_add_epi32(phase, inc);
  *bOut = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_insertf128_si256(_mm256_castsi128_si256(p), q, 1)), c);
#elif HV_SIMD_SSE
  static const __m128 c = (__m128) {__HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE, __HV_PHASOR_SCALE};
  __m128i phase = o->phase;
  *bOut = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srli_epi32(phase, 9)), c);
  o->phase = _mm_add_epi32(phase, o->inc);
#elif HV_SIMD_NEON
  uint32x4_t phase = o->phase;
  *bOut = vmulq_n_f32(vcvtq_f32_u32(vshrq_n_u32(phase, 9)), __HV_PHASOR_SCALE);
  o->phase = vaddq_u32(phase, vreinterpretq_u32_s32(o->inc));
#else // HV_SIMD_NONE
  *bOut = ((float) (o->phase >> 9)) * __HV_PHASOR_SCALE;
  o->phase += o->inc;
#endif
}

#endif // _HEAVY_SIGNAL_PHASOR_H_
