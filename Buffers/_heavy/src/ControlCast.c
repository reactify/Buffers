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

#include "ControlCast.h"

void cCast_onMessage(HvBase *_c, CastType castType, int letIn, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const)) {
  switch (castType) {
    case HV_CAST_BANG: {
      HvMessage *n = HV_MESSAGE_ON_STACK(1);
      msg_initWithBang(n, msg_getTimestamp(m));
      sendMessage(_c, 0, n);
      break;
    }
    case HV_CAST_FLOAT: {
      if (msg_isFloat(m, 0)) {
        HvMessage *n = HV_MESSAGE_ON_STACK(1);
        msg_initWithFloat(n, msg_getTimestamp(m), msg_getFloat(m, 0));
        sendMessage(_c, 0, n);
      }
      break;
    }
    case HV_CAST_SYMBOL: {
      switch (msg_getType(m, 0)) {
        case BANG: {
          HvMessage *n = HV_MESSAGE_ON_STACK(1);
          msg_initWithSymbol(n, msg_getTimestamp(m), "bang");
          sendMessage(_c, 0, n);
          break;
        }
        case FLOAT: {
          HvMessage *n = HV_MESSAGE_ON_STACK(1);
          msg_initWithSymbol(n, msg_getTimestamp(m), "float");
          sendMessage(_c, 0, n);
          break;
        }
        case SYMBOL: {
          sendMessage(_c, 0, m);
          break;
        }
        default: return;
      }
      break;
    }
    default: return;
  }
}
