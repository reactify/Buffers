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

#include "SignalSample.h"

#define __HV_SAMPLE_NULL -1

hv_size_t sSample_init(SignalSample *o) {
  o->i = __HV_SAMPLE_NULL;
  return 0;
}

void sSample_onMessage(HvBase *_c, SignalSample *o, int letIndex, const HvMessage *m) {
  o->i = msg_getTimestamp(m);
}

void __hv_sample_f(HvBase *_c, SignalSample *o, hv_bInf_t bIn,
    void (*sendMessage)(HvBase *, int, const HvMessage *)) {
  if (o->i != __HV_SAMPLE_NULL) {

#if HV_SIMD_AVX || HV_SIMD_SSE
    float out = bIn[o->i & HV_N_SIMD_MASK];
#elif HV_SIMD_NEON
    float out = bIn[o->i & HV_N_SIMD_MASK];
#else // HV_SIMD_NONE
    float out = bIn;
#endif

    HvMessage *n = HV_MESSAGE_ON_STACK(1);
    hv_uint32_t ts = (o->i + HV_N_SIMD) & ~HV_N_SIMD_MASK; // start of next block
    msg_initWithFloat(n, ts, out);
    ctx_scheduleMessage(_c, n, sendMessage, 0);
    o->i = __HV_SAMPLE_NULL; // reset the index
  }
}
