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

#include "SignalTabread.h"

hv_size_t sTabread_init(SignalTabread *o, HvTable *table, bool forceAlignedLoads) {
  o->table = table;
  o->head = 0;
  o->forceAlignedLoads = forceAlignedLoads;
  return 0;
}

void sTabread_onMessage(HvBase *_c, SignalTabread *o, int letIn, const HvMessage *const m) {
  switch (letIn) {
    case 0: {
      if (o->table != NULL) {
        switch (msg_getType(m,0)) {
          case BANG: o->head = 0; break;
          case FLOAT: {
            hv_uint32_t h = (hv_uint32_t) hv_abs_f(msg_getFloat(m,0));
            if (msg_getFloat(m,0) < 0.0f) {
              // if input is negative, wrap around the end of the table
              h = (hv_uint32_t) hTable_getSize(o->table) - h;
            }
            o->head = o->forceAlignedLoads ? h & ~HV_N_SIMD_MASK : h;
            break;
          }
          default: break;
        }
      }
      break;
    }
    case 1: {
      if (msg_isHashLike(m,0)) {
        o->table = ctx_getTableForHash(_c, msg_getHash(m,0));
      }
      break;
    }
    default: break;
  }
}



#if HV_APPLE
#pragma mark - Tabhead
#endif

void sTabhead_onMessage(HvBase *_c, SignalTabhead *o, const HvMessage *const m) {
  if (msg_isHashLike(m,0)) {
    o->table = ctx_getTableForHash(_c, msg_getHash(m,0));
  }
}

hv_size_t sTabhead_init(SignalTabhead *o, HvTable *table) {
  o->table = table;
  return 0;
}
