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

#include "SignalBiquad.h"

// http://reanimator-web.appspot.com/articles/simdiir
// http://musicdsp.org/files/Audio-EQ-Cookbook.txt

hv_size_t sBiquad_init(SignalBiquad *o) {
#if HV_SIMD_AVX
  o->xm1 = _mm256_setzero_ps();
  o->xm2 = _mm256_setzero_ps();
#elif HV_SIMD_SSE
  o->xm1 = _mm_setzero_ps();
  o->xm2 = _mm_setzero_ps();
#elif HV_SIMD_NEON
  o->xm1 = vdupq_n_f32(0.0f);
  o->xm2 = vdupq_n_f32(0.0f);
#else // HV_SIMD_NONE
  o->x1 = 0.0f;
  o->x2 = 0.0f;
#endif
  o->y1 = 0.0f;
  o->y2 = 0.0f;
  return 0;
}

void __hv_biquad_f(SignalBiquad *o, hv_bInf_t bIn, hv_bInf_t bX0, hv_bInf_t bX1, hv_bInf_t bX2, hv_bInf_t bY1, hv_bInf_t bY2, hv_bOutf_t bOut) {
#if HV_SIMD_AVX
  __m256 a = _mm256_mul_ps(bIn, bX0);
  __m256 b = _mm256_mul_ps(o->xm1, bX1);
  __m256 c = _mm256_mul_ps(o->xm2, bX2);
  __m256 d = _mm256_add_ps(a, b);
  __m256 e = _mm256_add_ps(c, d); // bIn*bX0 + o->x1*bX1 + o->x2*bX2
  float y0 = e[0] - o->y1*bY1[0] - o->y2*bY2[0];
  float y1 = e[1] - y0*bY1[1] - o->y1*bY2[1];
  float y2 = e[2] - y1*bY1[2] - y0*bY2[2];
  float y3 = e[3] - y2*bY1[3] - y1*bY2[3];
  float y4 = e[4] - y3*bY1[4] - y2*bY2[4];
  float y5 = e[5] - y4*bY1[5] - y3*bY2[5];
  float y6 = e[6] - y5*bY1[6] - y4*bY2[6];
  float y7 = e[7] - y6*bY1[7] - y5*bY2[7];

  o->xm2 = o->xm1;
  o->xm1 = bIn;
  o->y1 = y7;
  o->y2 = y6;

  *bOut = _mm256_set_ps(y7, y6, y5, y4, y3, y2, y1, y0);
#elif HV_SIMD_SSE
  __m128 a = _mm_mul_ps(bIn, bX0);
  __m128 b = _mm_mul_ps(o->xm1, bX1);
  __m128 c = _mm_mul_ps(o->xm2, bX2);
  __m128 d = _mm_add_ps(a, b);
  __m128 e = _mm_add_ps(c, d);
  float y0 = e[0] - o->y1*bY1[0] - o->y2*bY2[0];
  float y1 = e[1] - y0*bY1[1] - o->y1*bY2[1];
  float y2 = e[2] - y1*bY1[2] - y0*bY2[2];
  float y3 = e[3] - y2*bY1[3] - y1*bY2[3];

  o->xm2 = o->xm1;
  o->xm1 = bIn;
  o->y1 = y3;
  o->y2 = y2;

  *bOut = _mm_set_ps(y3, y2, y1, y0);
#elif HV_SIMD_NEON
  float32x4_t a = vmulq_f32(bIn, bX0);
  float32x4_t b = vmulq_f32(o->xm1, bX1);
  float32x4_t c = vmulq_f32(o->xm2, bX2);
  float32x4_t d = vaddq_f32(a, b);
  float32x4_t e = vaddq_f32(c, d);
  float y0 = e[0] - o->y1*bY1[0] - o->y2*bY2[0];
  float y1 = e[1] - y0*bY1[1] - o->y1*bY2[1];
  float y2 = e[2] - y1*bY1[2] - y0*bY2[2];
  float y3 = e[3] - y2*bY1[3] - y1*bY2[3];

  o->xm2 = o->xm1;
  o->xm1 = bIn;
  o->y1 = y3;
  o->y2 = y2;

  *bOut = (float32x4_t) {y0, y1, y2, y3};
#else
  const float y = bIn*bX0 + o->x1*bX1 + o->x2*bX2 - o->y1*bY1 - o->y2*bY2;
  o->x2 = o->x1; o->x1 = bIn;
  o->y2 = o->y1; o->y1 = y;
  *bOut = y;
#endif
}

static void sBiquad_k_updateCoefficients(SignalBiquad_k *const o) {
  // calculate all filter coefficients in the double domain
#if HV_SIMD_AVX || HV_SIMD_SSE || HV_SIMD_NEON
  double b0 = (double) o->b0;
  double b1 = (double) o->b1;
  double b2 = (double) o->b2;
  double a1 = (double) -o->a1;
  double a2 = (double) -o->a2;

  double coeffs[4][8] =
  {
    { 0,  0,  0,  b0, b1, b2, a1, a2 },
    { 0,  0,  b0, b1, b2, 0,  a2, 0  },
    { 0,  b0, b1, b2, 0,  0,  0,  0  },
    { b0, b1, b2, 0,  0,  0,  0,  0  },
  };

  for (int i = 0; i < 8; i++) {
    coeffs[1][i] += a1*coeffs[0][i];
    coeffs[2][i] += a1*coeffs[1][i] + a2*coeffs[0][i];
    coeffs[3][i] += a1*coeffs[2][i] + a2*coeffs[1][i];
  }

#if HV_SIMD_AVX || HV_SIMD_SSE
  o->coeff_xp3 = _mm_set_ps((float) coeffs[3][0], (float) coeffs[2][0], (float) coeffs[1][0], (float) coeffs[0][0]);
  o->coeff_xp2 = _mm_set_ps((float) coeffs[3][1], (float) coeffs[2][1], (float) coeffs[1][1], (float) coeffs[0][1]);
  o->coeff_xp1 = _mm_set_ps((float) coeffs[3][2], (float) coeffs[2][2], (float) coeffs[1][2], (float) coeffs[0][2]);
  o->coeff_x0 =  _mm_set_ps((float) coeffs[3][3], (float) coeffs[2][3], (float) coeffs[1][3], (float) coeffs[0][3]);
  o->coeff_xm1 = _mm_set_ps((float) coeffs[3][4], (float) coeffs[2][4], (float) coeffs[1][4], (float) coeffs[0][4]);
  o->coeff_xm2 = _mm_set_ps((float) coeffs[3][5], (float) coeffs[2][5], (float) coeffs[1][5], (float) coeffs[0][5]);
  o->coeff_ym1 = _mm_set_ps((float) coeffs[3][6], (float) coeffs[2][6], (float) coeffs[1][6], (float) coeffs[0][6]);
  o->coeff_ym2 = _mm_set_ps((float) coeffs[3][7], (float) coeffs[2][7], (float) coeffs[1][7], (float) coeffs[0][7]);
#else // HV_SIMD_NEON
  o->coeff_xp3 = (float32x4_t) {(float) coeffs[0][0], (float) coeffs[1][0], (float) coeffs[2][0], (float) coeffs[3][0]};
  o->coeff_xp2 = (float32x4_t) {(float) coeffs[0][1], (float) coeffs[1][1], (float) coeffs[2][1], (float) coeffs[3][1]};
  o->coeff_xp1 = (float32x4_t) {(float) coeffs[0][2], (float) coeffs[1][2], (float) coeffs[2][2], (float) coeffs[3][2]};
  o->coeff_x0 =  (float32x4_t) {(float) coeffs[0][3], (float) coeffs[1][3], (float) coeffs[2][3], (float) coeffs[3][3]};
  o->coeff_xm1 = (float32x4_t) {(float) coeffs[0][4], (float) coeffs[1][4], (float) coeffs[2][4], (float) coeffs[3][4]};
  o->coeff_xm2 = (float32x4_t) {(float) coeffs[0][5], (float) coeffs[1][5], (float) coeffs[2][5], (float) coeffs[3][5]};
  o->coeff_ym1 = (float32x4_t) {(float) coeffs[0][6], (float) coeffs[1][6], (float) coeffs[2][6], (float) coeffs[3][6]};
  o->coeff_ym2 = (float32x4_t) {(float) coeffs[0][7], (float) coeffs[1][7], (float) coeffs[2][7], (float) coeffs[3][7]};
#endif
#endif
  // NOTE(mhroth): not necessary to calculate any coefficients for HV_SIMD_NONE case
}

hv_size_t sBiquad_k_init(SignalBiquad_k *o, float b0, float b1, float b2, float a1, float a2) {
  // initialise filter coefficients
  o->b0 = b0;
  o->b1 = b1;
  o->b2 = b2;
  o->a1 = a1;
  o->a2 = a2;
  sBiquad_k_updateCoefficients(o);

  // clear filter state
#if HV_SIMD_AVX || HV_SIMD_SSE
  o->xm1 = _mm_setzero_ps();
  o->xm2 = _mm_setzero_ps();
  o->ym1 = _mm_setzero_ps();
  o->ym2 = _mm_setzero_ps();
#elif HV_SIMD_NEON
  o->xm1 = vdupq_n_f32(0.0f);
  o->xm2 = vdupq_n_f32(0.0f);
  o->ym1 = vdupq_n_f32(0.0f);
  o->ym2 = vdupq_n_f32(0.0f);
#else // HV_SIMD_NONE
  o->xm1 = 0.0f;
  o->xm2 = 0.0f;
  o->ym1 = 0.0f;
  o->ym2 = 0.0f;
#endif
  return 0;
}

void sBiquad_k_onMessage(SignalBiquad_k *o, int letIn, const HvMessage *const m) {
  if (msg_isFloat(m,0)) {
    switch (letIn) {
      case 1: o->b0 = msg_getFloat(m,0); break;
      case 2: o->b1 = msg_getFloat(m,0); break;
      case 3: o->b2 = msg_getFloat(m,0); break;
      case 4: o->a1 = msg_getFloat(m,0); break;
      case 5: o->a2 = msg_getFloat(m,0); break;
      default: return;
    }
    sBiquad_k_updateCoefficients(o);
  }
}
