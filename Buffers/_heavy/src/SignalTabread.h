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

#ifndef _HEAVY_SIGNAL_TABREAD_H_
#define _HEAVY_SIGNAL_TABREAD_H_

#include "HvBase.h"
#include "HvTable.h"

#if HV_APPLE
#pragma mark - Tabread - Random Access
#endif

typedef struct SignalTabread {
  HvTable *table; // the table to read
  hv_uint32_t head;
  bool forceAlignedLoads; // false by default, true if using __hv_tabread_f
} SignalTabread;

// random access to a table
hv_size_t sTabread_init(SignalTabread *o, HvTable *table, bool forceAlignedLoads);

static inline void __hv_tabread_if(SignalTabread *o, hv_bIni_t bIn, hv_bOutf_t bOut) {
  const float *const b = hTable_getBuffer(o->table);
#if HV_SIMD_AVX
  hv_assert((int) (bIn[0] & 0xFFFFFFFFL) >= 0 && (int) (bIn[0] & 0xFFFFFFFFL) < hTable_getSize(o->table));
  hv_assert((int) ((bIn[0] & ~0xFFFFFFFFL) >> 32) >= 0 && (int) ((bIn[0] & ~0xFFFFFFFFL) >> 32) < hTable_getSize(o->table));
  hv_assert((int) (bIn[1] & 0xFFFFFFFFL) >= 0 && (int) (bIn[1] & 0xFFFFFFFFL) < hTable_getSize(o->table));
  hv_assert((int) ((bIn[1] & ~0xFFFFFFFFL) >> 32) >= 0 && (int) ((bIn[1] & ~0xFFFFFFFFL) >> 32) < hTable_getSize(o->table));
  hv_assert((int) (bIn[2] & 0xFFFFFFFFL) >= 0 && (int) (bIn[2] & 0xFFFFFFFFL) < hTable_getSize(o->table));
  hv_assert((int) ((bIn[2] & ~0xFFFFFFFFL) >> 32) >= 0 && (int) ((bIn[2] & ~0xFFFFFFFFL) >> 32) < hTable_getSize(o->table));
  hv_assert((int) (bIn[3] & 0xFFFFFFFFL) >= 0 && (int) (bIn[3] & 0xFFFFFFFFL) < hTable_getSize(o->table));
  hv_assert((int) ((bIn[3] & ~0xFFFFFFFFL) >> 32) >= 0 && (int) ((bIn[3] & ~0xFFFFFFFFL) >> 32) < hTable_getSize(o->table));

  *bOut = _mm256_set_ps(
      b[(int) ((bIn[3] & ~0xFFFFFFFFL) >> 32)],
      b[(int) (bIn[3] & 0xFFFFFFFFL)],
      b[(int) ((bIn[2] & ~0xFFFFFFFFL) >> 32)],
      b[(int) (bIn[2] & 0xFFFFFFFFL)],
      b[(int) ((bIn[1] & ~0xFFFFFFFFL) >> 32)],
      b[(int) (bIn[1] & 0xFFFFFFFFL)],
      b[(int) ((bIn[0] & ~0xFFFFFFFFL) >> 32)],
      b[(int) (bIn[0] & 0xFFFFFFFFL)]);
#elif HV_SIMD_SSE
  hv_assert((int) (bIn[0] & 0xFFFFFFFFL) >= 0 && (int) (bIn[0] & 0xFFFFFFFFL) < hTable_getSize(o->table));
  hv_assert((int) ((bIn[0] & ~0xFFFFFFFFL) >> 32) >= 0 && (int) ((bIn[0] & ~0xFFFFFFFFL) >> 32) < hTable_getSize(o->table));
  hv_assert((int) (bIn[1] & 0xFFFFFFFFL) >= 0 && (int) (bIn[1] & 0xFFFFFFFFL) < hTable_getSize(o->table));
  hv_assert((int) ((bIn[1] & ~0xFFFFFFFFL) >> 32) >= 0 && (int) ((bIn[1] & ~0xFFFFFFFFL) >> 32) < hTable_getSize(o->table));

  *bOut = _mm_set_ps(
      b[(int) ((bIn[1] & ~0xFFFFFFFFL) >> 32)],
      b[(int) (bIn[1] & 0xFFFFFFFFL)],
      b[(int) ((bIn[0] & ~0xFFFFFFFFL) >> 32)],
      b[(int) (bIn[0] & 0xFFFFFFFFL)]);
#elif HV_SIMD_NEON
  hv_assert((bIn[0] >= 0) && (bIn[0] < hTable_getSize(o->table)));
  hv_assert((bIn[1] >= 0) && (bIn[1] < hTable_getSize(o->table)));
  hv_assert((bIn[2] >= 0) && (bIn[2] < hTable_getSize(o->table)));
  hv_assert((bIn[3] >= 0) && (bIn[3] < hTable_getSize(o->table)));

  *bOut = (float32x4_t) {b[bIn[0]], b[bIn[1]], b[bIn[2]], b[bIn[3]]};
#else // HV_SIMD_NONE
  hv_assert(bIn >= 0 && ((hv_uint32_t) bIn < hTable_getSize(o->table)));

  *bOut = b[bIn];
#endif
}



#if HV_APPLE
#pragma mark - Tabread - Linear Access
#endif

// this tabread never stops reading. It is mainly intended for linear reads that loop around a table.
static inline void __hv_tabread_f(SignalTabread *o, hv_bOutf_t bOut) {
  hv_assert((o->head + HV_N_SIMD) <= hTable_getSize(o->table)); // assert that we always read within the table bounds
  hv_uint32_t head = o->head;
#if HV_SIMD_AVX
  *bOut = _mm256_load_ps(hTable_getBuffer(o->table) + head);
#elif HV_SIMD_SSE
  *bOut = _mm_load_ps(hTable_getBuffer(o->table) + head);
#elif HV_SIMD_NEON
  *bOut = vld1q_f32(hTable_getBuffer(o->table) + head);
#else // HV_SIMD_NONE
  *bOut = *(hTable_getBuffer(o->table) + head);
#endif
  o->head = head + HV_N_SIMD;
}

// unaligned linear tabread, as above
static inline void __hv_tabreadu_f(SignalTabread *o, hv_bOutf_t bOut) {
  hv_assert((o->head + HV_N_SIMD) < hTable_getSize(o->table)); // assert that we always read within the table bounds
  hv_uint32_t head = o->head;
#if HV_SIMD_AVX
  *bOut = _mm256_loadu_ps(hTable_getBuffer(o->table) + head);
#elif HV_SIMD_SSE
  *bOut = _mm_loadu_ps(hTable_getBuffer(o->table) + head);
#elif HV_SIMD_NEON
  *bOut = vld1q_f32(hTable_getBuffer(o->table) + head);
#else // HV_SIMD_NONE
  *bOut = *(hTable_getBuffer(o->table) + head);
#endif
  o->head = head + HV_N_SIMD;
}

// this tabread can be instructed to stop. It is mainly intended for linear reads that only process a portion of a buffer.
static inline void __hv_tabread_stoppable_f(SignalTabread *o, hv_bOutf_t bOut) {
#if HV_SIMD_AVX
  if (o->head == ~0x0) {
    *bOut = _mm256_setzero_ps();
  } else {
    *bOut = _mm256_load_ps(hTable_getBuffer(o->table) + o->head);
    o->head += HV_N_SIMD;
  }
#elif HV_SIMD_SSE
  if (o->head == ~0x0) {
    *bOut = _mm_setzero_ps();
  } else {
    *bOut = _mm_load_ps(hTable_getBuffer(o->table) + o->head);
    o->head += HV_N_SIMD;
  }
#elif HV_SIMD_NEON
  if (o->head == ~0x0) {
    *bOut = vdupq_n_f32(0.0f);
  } else {
    *bOut = vld1q_f32(hTable_getBuffer(o->table) + o->head);
    o->head += HV_N_SIMD;
  }
#else // HV_SIMD_NONE
  if (o->head == ~0x0) {
    *bOut = 0.0f;
  } else {
    *bOut = *(hTable_getBuffer(o->table) + o->head);
    o->head += HV_N_SIMD;
  }
#endif
}

void sTabread_onMessage(HvBase *_c, SignalTabread *o, int letIn, const HvMessage *const m);



#if HV_APPLE
#pragma mark - Tabhead
#endif

typedef struct SignalTabhead {
  HvTable *table;
} SignalTabhead;

hv_size_t sTabhead_init(SignalTabhead *o, HvTable *table);

static inline void __hv_tabhead_f(SignalTabhead *o, hv_bOutf_t bOut) {
#if HV_SIMD_AVX
  *bOut = _mm256_set1_ps((float) hTable_getHead(o->table));
#elif HV_SIMD_SSE
  *bOut = _mm_set1_ps((float) hTable_getHead(o->table));
#elif HV_SIMD_NEON
  *bOut = vdupq_n_f32((float32_t) hTable_getHead(o->table));
#else // HV_SIMD_NONE
  *bOut = (float) hTable_getHead(o->table);
#endif
}

void sTabhead_onMessage(HvBase *_c, SignalTabhead *o, const HvMessage *const m);

#endif // _HEAVY_SIGNAL_TABREAD_H_
