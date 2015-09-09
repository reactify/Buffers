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
 *
 * DO NOT MODIFY. THIS CODE IS MACHINE GENERATED BY THE SECTION6 HEAVY COMPILER.
 */

/*
 * System Includes
 */

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "HvContext_Playhead.h"
#include "HeavyMath.h"


/*
 * Function Declarations
 */
static void cReceive_AADZ3_sendMessage(HvBase *, int, const HvMessage *const);
static void cBinop_b241h_sendMessage(HvBase *, int, const HvMessage *const);
static void cBinop_GpEoZ_sendMessage(HvBase *, int, const HvMessage *const);
static void cVar_lJlm3_sendMessage(HvBase *, int, const HvMessage *const);
static void cCast_V7hO9_sendMessage(HvBase *, int, const HvMessage *const);
static void cCast_GtWQ2_sendMessage(HvBase *, int, const HvMessage *const);
static void cCast_EEF2S_sendMessage(HvBase *, int, const HvMessage *const);
static void cCast_4FT5g_sendMessage(HvBase *, int, const HvMessage *const);
static void cReceive_Dc1XW_sendMessage(HvBase *, int, const HvMessage *const);



/*
 * Static Helper Functions
 */

static void ctx_intern_scheduleMessageForReceiver(
    HvBase *const _c, const char *name, HvMessage *m) {
  switch (msg_symbolToHash(name)) {
    case 0x9A248230: { // Speed
      ctx_scheduleMessage(_c, m, &cReceive_AADZ3_sendMessage, 0);
      break;
    }
    case 0xBAE67279: { // SetBufferSize
      ctx_scheduleMessage(_c, m, &cReceive_Dc1XW_sendMessage, 0);
      break;
    }
    default: return;
  }
}

static struct HvTable *ctx_intern_getTableForHash(HvBase *const _c, hv_uint32_t h) {
  switch (h) {
    default: return NULL;
  }
}



/*
 * Context Include and Implementatons
 */

Hv_Playhead *hv_Playhead_new_with_pool(double sampleRate, int poolKb) {
  hv_assert(sampleRate > 0.0); // can't initialise with sampling rate of 0
  hv_assert(poolKb >= 1); // a message pool of some reasonable size is always needed
  Hv_Playhead *const _c = (Hv_Playhead *) hv_malloc(sizeof(Hv_Playhead));

  Base(_c)->numInputChannels = 0;
  Base(_c)->numOutputChannels = 1;
  Base(_c)->sampleRate = sampleRate;
  Base(_c)->blockStartTimestamp = 0;
  Base(_c)->f_scheduleMessageForReceiver = &ctx_intern_scheduleMessageForReceiver;
  Base(_c)->f_getTableForHash = &ctx_intern_getTableForHash;
  mq_initWithPoolSize(&Base(_c)->mq, poolKb);
  Base(_c)->basePath = NULL;
  Base(_c)->printHook = NULL;
  Base(_c)->sendHook = NULL;
  Base(_c)->userData = NULL;
  Base(_c)->name = "Playhead";

  Base(_c)->numBytes = sizeof(Hv_Playhead);
  Base(_c)->numBytes += sVarf_init(&_c->sVarf_ub0CS, 1.0f, 0.0f, false);
  Base(_c)->numBytes += sPhasor_init(&_c->sPhasor_0Bvx5, sampleRate);
  Base(_c)->numBytes += cBinop_init(&_c->cBinop_b241h, 1.0f); // __mul
  Base(_c)->numBytes += cBinop_init(&_c->cBinop_GpEoZ, 0.0f); // __div
  Base(_c)->numBytes += cVar_init_f(&_c->cVar_lJlm3, 44100.0f);

  // loadbang

  return _c;
}

Hv_Playhead *hv_Playhead_new(double sampleRate) {
  return hv_Playhead_new_with_pool(sampleRate, 10); // default to 10KB MessagePool
}

void hv_Playhead_free(Hv_Playhead *_c) {

  hv_free(Base(_c)->basePath);
  mq_free(&Base(_c)->mq); // free queue after all objects have been freed, messages may be cancelled

  hv_free(_c);
}



/*
 * Static Function Implementation
 */
static void cReceive_AADZ3_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_b241h, HV_BINOP_MULTIPLY, 0, m, &cBinop_b241h_sendMessage);
}

static void cBinop_b241h_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  sVarf_onMessage(_c, &Context(_c)->sVarf_ub0CS, m);
}

static void cBinop_GpEoZ_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cCast_onMessage(_c, HV_CAST_FLOAT, 0, m, &cCast_GtWQ2_sendMessage);
  cCast_onMessage(_c, HV_CAST_BANG, 0, m, &cCast_V7hO9_sendMessage);
}

static void cVar_lJlm3_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_GpEoZ, HV_BINOP_DIVIDE, 0, m, &cBinop_GpEoZ_sendMessage);
}

static void cCast_V7hO9_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_b241h, HV_BINOP_MULTIPLY, 0, m, &cBinop_b241h_sendMessage);
}

static void cCast_GtWQ2_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_b241h, HV_BINOP_MULTIPLY, 1, m, &cBinop_b241h_sendMessage);
}

static void cCast_EEF2S_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_GpEoZ, HV_BINOP_DIVIDE, 1, m, &cBinop_GpEoZ_sendMessage);
}

static void cCast_4FT5g_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cVar_onMessage(_c, &Context(_c)->cVar_lJlm3, 0, m, &cVar_lJlm3_sendMessage);
}

static void cReceive_Dc1XW_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cCast_onMessage(_c, HV_CAST_FLOAT, 0, m, &cCast_EEF2S_sendMessage);
  cCast_onMessage(_c, HV_CAST_BANG, 0, m, &cCast_4FT5g_sendMessage);
}




/*
 * Context Process Implementation
 */

int hv_Playhead_process(Hv_Playhead *const _c, float **const inputBuffers, float **const outputBuffers, int nx) {
  const int n4 = nx & ~HV_N_SIMD_MASK; // ensure that the block size is a multiple of HV_N_SIMD

  // temporary signal vars
  hv_bufferf_t Bf0;

  // input and output vars
  hv_bufferf_t O0;

  // declare and init the zero buffer
  hv_bufferf_t ZERO; __hv_zero_f(VOf(ZERO));

  hv_uint32_t nextBlock = Base(_c)->blockStartTimestamp;
  for (int n = 0; n < n4; n += HV_N_SIMD) {

    // process all of the messages for this block
    nextBlock += HV_N_SIMD;
    while (mq_hasMessageBefore(&Base(_c)->mq, nextBlock)) {
      MessageNode *const node = mq_peek(&Base(_c)->mq);
      node->sendMessage(Base(_c), node->let, node->m);
      mq_pop(&Base(_c)->mq);
    }

    // zero output buffers
    __hv_zero_f(VOf(O0));

    // process all signal functions
    __hv_var_f(&_c->sVarf_ub0CS, VOf(Bf0));
    __hv_phasor_f(&_c->sPhasor_0Bvx5, VIf(Bf0), VOf(Bf0));
    __hv_add_f(VIf(Bf0), VIf(O0), VOf(O0));

    // save output vars to output buffer
    __hv_store_f(outputBuffers[0]+n, VIf(O0));
  }

  Base(_c)->blockStartTimestamp = nextBlock;

  return n4; // return the number of frames processed
}

int hv_Playhead_process_inline(Hv_Playhead *c, float *const inputBuffers, float *const outputBuffers, int n4) {
  hv_assert(!(n4 & HV_N_SIMD_MASK)); // ensure that n4 is a multiple of HV_N_SIMD
  int i = ctx_getNumInputChannels(Base(c));
  float **bIn = (float **) hv_alloca(i*sizeof(float *));
  while (i--) bIn[i] = inputBuffers+(i*n4);

  i = ctx_getNumOutputChannels(Base(c));
  float **bOut = (float **) hv_alloca(i*sizeof(float *));
  while (i--) bOut[i] = outputBuffers+(i*n4);

  int n = hv_Playhead_process(c, bIn, bOut, n4);
  return n;
}

int hv_Playhead_process_inline_short(Hv_Playhead *c, short *const inputBuffers, short *const outputBuffers, int n4) {
  hv_assert(!(n4 & HV_N_SIMD_MASK)); // ensure that n4 is a multiple of HV_N_SIMD
  int numChannels = ctx_getNumInputChannels(Base(c));
  float *bIn = (float *) hv_alloca(numChannels*n4*sizeof(float));
  for (int i = 0; i < numChannels; ++i) {
    for (int j = 0; j < n4; ++j) {
      bIn[i*n4+j] = ((float) inputBuffers[i+numChannels*j]) * 0.00003051757813f;
    }
  }

  numChannels = ctx_getNumOutputChannels(Base(c));
  float *bOut = (float *) hv_alloca(numChannels*n4*sizeof(float));

  int n = hv_Playhead_process_inline(c, bIn, bOut, n4);

  for (int i = 0; i < numChannels; ++i) {
    for (int j = 0; j < n4; ++j) {
      outputBuffers[i+numChannels*j] = (short) (bOut[i*n4+j] * 32767.0f);
    }
  }

  return n;
}