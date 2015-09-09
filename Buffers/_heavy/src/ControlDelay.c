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

#include "ControlDelay.h"

hv_size_t cDelay_init(HvBase *_c, ControlDelay *o, float delayMs) {
  o->delay = ctx_millisecondsToSamples(_c, delayMs);
  hv_memset(o->msgs, __HV_DELAY_MAX_MESSAGES*sizeof(HvMessage *));
  return 0;
}

void cDelay_onMessage(HvBase *_c, ControlDelay *o, int letIn, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const)) {
  switch (letIn) {
    case 0: {
      if (msg_compareSymbol(m, 0, "flush")) {
        // send all messages immediately
        for (int i = 0; i < __HV_DELAY_MAX_MESSAGES; i++) {
          HvMessage *n = o->msgs[i];
          if (n != NULL) {
            msg_setTimestamp(n, msg_getTimestamp(m)); // update the timestamp to now
            sendMessage(_c, 0, n); // send the message
            ctx_cancelMessage(_c, n, sendMessage); // then clear it
            // NOTE(mhroth): there may be a problem here if a flushed message causes a clear message to return
            // to this object in the same step
          }
        }
        hv_memset(o->msgs, __HV_DELAY_MAX_MESSAGES*sizeof(HvMessage *));
      } else if (msg_compareSymbol(m, 0, "clear")) {
        // cancel (clear) all (pending) messages
        for (int i = 0; i < __HV_DELAY_MAX_MESSAGES; i++) {
          HvMessage *n = o->msgs[i];
          if (n != NULL) {
            ctx_cancelMessage(_c, n, sendMessage);
          }
        }
        hv_memset(o->msgs, __HV_DELAY_MAX_MESSAGES*sizeof(HvMessage *));
      } else {
        hv_uint32_t ts = msg_getTimestamp(m);
        msg_setTimestamp((HvMessage *) m, ts+o->delay); // update the timestamp to set the delay
        int i;
        for (i = 0; i < __HV_DELAY_MAX_MESSAGES; i++) {
          if (o->msgs[i] == NULL) {
            o->msgs[i] = ctx_scheduleMessage(_c, m, sendMessage, 0);
            break;
          }
        }
        hv_assert(i < __HV_DELAY_MAX_MESSAGES); // scheduled message limit reached
        msg_setTimestamp((HvMessage *) m, ts); // return to the original timestamp
      }
      break;
    }
    case 1: {
      if (msg_isFloat(m,0)) {
        // set delay in milliseconds
        o->delay = ctx_millisecondsToSamples(_c,msg_getFloat(m,0));
      }
      break;
    }
    case 2: {
      if (msg_isFloat(m,0)) {
        // set delay in samples
        o->delay = (hv_uint32_t) msg_getFloat(m,0);
      }
      break;
    }
    default: break;
  }
}

void cDelay_clearExecutingMessage(ControlDelay *o, const HvMessage *const m) {
  for (int i = 0; i < __HV_DELAY_MAX_MESSAGES; ++i) {
    if (o->msgs[i] == m) {
      o->msgs[i] = NULL;
      break;
    }
  }
}
