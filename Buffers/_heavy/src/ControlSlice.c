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

#include "ControlSlice.h"

hv_size_t cSlice_init(ControlSlice *o, int i, int n) {
  o->i = i;
  o->n = n;
  return 0;
}

void cSlice_onMessage(HvBase *_c, ControlSlice *o, int letIn, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const)) {
  switch (letIn) {
    case 0: {
      // if the start point is greater than the number of elements in the source message, do nothing
      if (o->i < msg_getNumElements(m)) {
        int x = msg_getNumElements(m) - o->i; // number of elements in the new message
        if (o->n > 0) x = hv_min_i(x, o->n);
        HvMessage *n = HV_MESSAGE_ON_STACK(x);
        msg_init(n, x, msg_getTimestamp(m));
        hv_memcpy(&n->elem, &m->elem+o->i, x*sizeof(Element));
        sendMessage(_c, 0, n);
      } else {
        // if nothing can be sliced, send a bang out of the right outlet
        HvMessage *n = HV_MESSAGE_ON_STACK(1);
        msg_initWithBang(n, msg_getTimestamp(m));
        sendMessage(_c, 1, n);
      }
      break;
    }
    case 1: {
      if (msg_isFloat(m,0)) {
        o->i = (int) msg_getFloat(m,0);
        if (msg_isFloat(m,1)) {
          o->n = (int) msg_getFloat(m,1);
        }
      }
      break;
    }
    case 2: {
      if (msg_isFloat(m,0)) {
        o->n = (int) msg_getFloat(m,0);
      }
      break;
    }
    default: break;
  }
}
