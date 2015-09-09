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

#ifndef _HEAVY_SIGNAL_BIQUAD_H_
#define _HEAVY_SIGNAL_BIQUAD_H_

#include "HvBase.h"

// http://en.wikipedia.org/wiki/Digital_biquad_filter
typedef struct SignalBiquad {
#if HV_SIMD_AVX
  __m256 xm1;
  __m256 xm2;
#elif HV_SIMD_SSE
  __m128 xm1;
  __m128 xm2;
#elif HV_SIMD_NEON
  float32x4_t xm1;
  float32x4_t xm2;
#else // HV_SIMD_NONE
  float x1;
  float x2;
#endif
  float y1;
  float y2;
} SignalBiquad;

hv_size_t sBiquad_init(SignalBiquad *o);

void __hv_biquad_f(SignalBiquad *o,
    hv_bInf_t bIn, hv_bInf_t bX0, hv_bInf_t bX1, hv_bInf_t bX2, hv_bInf_t bY1, hv_bInf_t bY2,
    hv_bOutf_t bOut);

typedef struct SignalBiquad_k {
#if HV_SIMD_AVX || HV_SIMD_SSE
  // preprocessed filter coefficients
  __m128 coeff_xp3;
  __m128 coeff_xp2;
  __m128 coeff_xp1;
  __m128 coeff_x0;
  __m128 coeff_xm1;
  __m128 coeff_xm2;
  __m128 coeff_ym1;
  __m128 coeff_ym2;

  // filter state
  __m128 xm1;
  __m128 xm2;
  __m128 ym1;
  __m128 ym2;
#elif HV_SIMD_NEON
  float32x4_t coeff_xp3;
  float32x4_t coeff_xp2;
  float32x4_t coeff_xp1;
  float32x4_t coeff_x0;
  float32x4_t coeff_xm1;
  float32x4_t coeff_xm2;
  float32x4_t coeff_ym1;
  float32x4_t coeff_ym2;
  float32x4_t xm1;
  float32x4_t xm2;
  float32x4_t ym1;
  float32x4_t ym2;
#else // HV_SIMD_NONE
  float xm1;
  float xm2;
  float ym1;
  float ym2;
#endif
  // original filter coefficients
  float b0; // x[0]
  float b1; // x[-1]
  float b2; // x[-2]
  float a1; // y[-1]
  float a2; // y[-2]
} SignalBiquad_k;

hv_size_t sBiquad_k_init(SignalBiquad_k *o, float x0, float x1, float x2, float y1, float y2);

void sBiquad_k_onMessage(SignalBiquad_k *o, int letIn, const HvMessage *const m);

static inline void __hv_biquad_k_f(SignalBiquad_k *o, hv_bInf_t bIn, hv_bOutf_t bOut) {
#if HV_SIMD_AVX
  const __m128 c_xp3 = o->coeff_xp3;
  const __m128 c_xp2 = o->coeff_xp2;
  const __m128 c_xp1 = o->coeff_xp1;
  const __m128 c_x0 = o->coeff_x0;
  const __m128 c_xm1 = o->coeff_xm1;
  const __m128 c_xm2 = o->coeff_xm2;
  const __m128 c_ym1 = o->coeff_ym1;
  const __m128 c_ym2 = o->coeff_ym2;

  // lower half
  __m128 x3 = _mm_set1_ps(bIn[3]);
  __m128 x2 = _mm_set1_ps(bIn[2]);
  __m128 x1 = _mm_set1_ps(bIn[1]);
  __m128 x0 = _mm_set1_ps(bIn[0]);
  __m128 xm1 = o->xm1;
  __m128 xm2 = o->xm2;
  __m128 ym1 = o->ym1;
  __m128 ym2 = o->ym2;

  __m128 a = _mm_mul_ps(c_xp3, x3);
  __m128 b = _mm_mul_ps(c_xp2, x2);
  __m128 c = _mm_mul_ps(c_xp1, x1);
  __m128 d = _mm_mul_ps(c_x0, x0);
  __m128 e = _mm_mul_ps(c_xm1, xm1);
  __m128 f = _mm_mul_ps(c_xm2, xm2);
  __m128 g = _mm_mul_ps(c_ym1, ym1);
  __m128 h = _mm_mul_ps(c_ym2, ym2);

  __m128 i = _mm_add_ps(a, b);
  __m128 j = _mm_add_ps(c, d);
  __m128 k = _mm_add_ps(e, f);
  __m128 l = _mm_add_ps(g, h);
  __m128 m = _mm_add_ps(i, j);
  __m128 n = _mm_add_ps(k, l);

  __m128 lo_y = _mm_add_ps(m, n); // lower part of output buffer

  // upper half
  xm1 = x3;
  xm2 = x2;
  x3 = _mm_set1_ps(bIn[7]);
  x2 = _mm_set1_ps(bIn[6]);
  x1 = _mm_set1_ps(bIn[5]);
  x0 = _mm_set1_ps(bIn[4]);
  ym1 = _mm_set1_ps(lo_y[3]);
  ym2 = _mm_set1_ps(lo_y[2]);

  a = _mm_mul_ps(c_xp3, x3);
  b = _mm_mul_ps(c_xp2, x2);
  c = _mm_mul_ps(c_xp1, x1);
  d = _mm_mul_ps(c_x0, x0);
  e = _mm_mul_ps(c_xm1, xm1);
  f = _mm_mul_ps(c_xm2, xm2);
  g = _mm_mul_ps(c_ym1, ym1);
  h = _mm_mul_ps(c_ym2, ym2);

  i = _mm_add_ps(a, b);
  j = _mm_add_ps(c, d);
  k = _mm_add_ps(e, f);
  l = _mm_add_ps(g, h);
  m = _mm_add_ps(i, j);
  n = _mm_add_ps(k, l);

  __m128 up_y = _mm_add_ps(m, n); // upper part of output buffer

  o->xm1 = x3;
  o->xm2 = x2;
  o->ym1 = _mm_set1_ps(up_y[3]);
  o->ym2 = _mm_set1_ps(up_y[2]);

  *bOut = _mm256_insertf128_ps(_mm256_castps128_ps256(lo_y), up_y, 1);
#elif HV_SIMD_SSE
  __m128 x3 = _mm_set1_ps(bIn[3]);
  __m128 x2 = _mm_set1_ps(bIn[2]);
  __m128 x1 = _mm_set1_ps(bIn[1]);
  __m128 x0 = _mm_set1_ps(bIn[0]);

  __m128 a = _mm_mul_ps(o->coeff_xp3, x3);
  __m128 b = _mm_mul_ps(o->coeff_xp2, x2);
  __m128 c = _mm_mul_ps(o->coeff_xp1, x1);
  __m128 d = _mm_mul_ps(o->coeff_x0, x0);
  __m128 e = _mm_mul_ps(o->coeff_xm1, o->xm1);
  __m128 f = _mm_mul_ps(o->coeff_xm2, o->xm2);
  __m128 g = _mm_mul_ps(o->coeff_ym1, o->ym1);
  __m128 h = _mm_mul_ps(o->coeff_ym2, o->ym2);
  __m128 i = _mm_add_ps(a, b);
  __m128 j = _mm_add_ps(c, d);
  __m128 k = _mm_add_ps(e, f);
  __m128 l = _mm_add_ps(g, h);
  __m128 m = _mm_add_ps(i, j);
  __m128 n = _mm_add_ps(k, l);

  __m128 y = _mm_add_ps(m, n);

  o->xm1 = x3;
  o->xm2 = x2;
  o->ym1 = _mm_set1_ps(y[3]);
  o->ym2 = _mm_set1_ps(y[2]);

  *bOut = y;
#elif HV_SIMD_NEON
  float32x4_t x3 = vdupq_n_f32(bIn[3]);
  float32x4_t x2 = vdupq_n_f32(bIn[2]);
  float32x4_t x1 = vdupq_n_f32(bIn[1]);
  float32x4_t x0 = vdupq_n_f32(bIn[0]);

  float32x4_t a = vmulq_f32(o->coeff_xp3, x3);
  float32x4_t b = vmulq_f32(o->coeff_xp2, x2);
  float32x4_t c = vmulq_f32(o->coeff_xp1, x1);
  float32x4_t d = vmulq_f32(o->coeff_x0, x0);
  float32x4_t e = vmulq_f32(o->coeff_xm1, o->xm1);
  float32x4_t f = vmulq_f32(o->coeff_xm2, o->xm2);
  float32x4_t g = vmulq_f32(o->coeff_ym1, o->ym1);
  float32x4_t h = vmulq_f32(o->coeff_ym2, o->ym2);
  float32x4_t i = vaddq_f32(a, b);
  float32x4_t j = vaddq_f32(c, d);
  float32x4_t k = vaddq_f32(e, f);
  float32x4_t l = vaddq_f32(g, h);
  float32x4_t m = vaddq_f32(i, j);
  float32x4_t n = vaddq_f32(k, l);
  float32x4_t y = vaddq_f32(m, n);

  o->xm1 = x3;
  o->xm2 = x2;
  o->ym1 = vdupq_n_f32(y[3]);
  o->ym2 = vdupq_n_f32(y[2]);

  *bOut = y;
#else // HV_SIMD_NONE
  float y = o->b0*bIn + o->b1*o->xm1 + o->b2*o->xm2 - o->a1*o->ym1 - o->a2*o->ym2;
  o->xm2 = o->xm1;
  o->xm1 = bIn;
  o->ym2 = o->ym1;
  o->ym1 = y;
  *bOut = y;
#endif
}

#endif // _HEAVY_SIGNAL_BIQUAD_H_
