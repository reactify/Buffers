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

#ifndef _HEAVY_SIGNAL_VAR_H_
#define _HEAVY_SIGNAL_VAR_H_

#include "HvBase.h"

// __var~f
// __varset~f

typedef struct SignalVarf {
  hv_bufferf_t v;
} SignalVarf;

hv_size_t sVarf_init(SignalVarf *o, float k, float step, bool reverse);

static inline void __hv_var_f(SignalVarf *o, hv_bOutf_t bOut) {
  *bOut = o->v;
}

static inline void sVarsetf_process(SignalVarf *o, hv_bInf_t bIn) {
  o->v = bIn;
}

void sVarf_onMessage(HvBase *_c, SignalVarf *o, const HvMessage *m);



// __var~i
// __varset~i

typedef struct SignalVari {
  hv_bufferi_t v;
} SignalVari;

hv_size_t sVari_init(SignalVari *o, int k, int step, bool reverse);

static inline void __hv_var_i(SignalVari *o, hv_bOuti_t bOut) {
  *bOut = o->v;
}

#if HV_SIMD_AVX
#define __hv_var_k_i_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m256i) {_a,_b,_c,_d,_e,_f,_g,_h})
#define __hv_var_k_i_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m256i) {_h,_g,_f,_e,_d,_c,_b,_a})
#define __hv_var_k_f_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m256) {_a,_b,_c,_d,_e,_f,_g,_h})
#define __hv_var_k_f_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m256) {_h,_g,_f,_e,_d,_c,_b,_a})
#elif HV_SIMD_SSE
#define __hv_var_k_i_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m128i) {_a,_b,_c,_d})
#define __hv_var_k_i_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m128i) {_h,_g,_f,_e})
#define __hv_var_k_f_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m128) {_a,_b,_c,_d})
#define __hv_var_k_f_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((__m128) {_h,_g,_f,_e})
#elif HV_SIMD_NEON
#define __hv_var_k_i_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((int32x4_t) {_a,_b,_c,_d})
#define __hv_var_k_i_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((int32x4_t) {_h,_g,_f,_e})
#define __hv_var_k_f_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((float32x4_t) {_a,_b,_c,_d})
#define __hv_var_k_f_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=((float32x4_t) {_h,_g,_f,_e})
#else // HV_SIMD_NONE
#define __hv_var_k_i_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=_a
#define __hv_var_k_i_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=_a
#define __hv_var_k_f_0(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=_a
#define __hv_var_k_f_1(_z,_a,_b,_c,_d,_e,_f,_g,_h) *_z=_a
#endif
// r == 0: forwards, r == 1: backwards
#define __hv_var_k_i(_z,_a,_b,_c,_d,_e,_f,_g,_h,_r) __hv_var_k_i_##_r(_z,_a,_b,_c,_d,_e,_f,_g,_h)
#define __hv_var_k_f(_z,_a,_b,_c,_d,_e,_f,_g,_h,_r) __hv_var_k_f_##_r(_z,_a,_b,_c,_d,_e,_f,_g,_h)

static inline void sVarseti_process(SignalVari *o, hv_bIni_t bIn) {
  o->v = bIn;
}

void sVari_onMessage(HvBase *_c, SignalVari *o, const HvMessage *m);

#endif // _HEAVY_SIGNAL_VAR_H_
