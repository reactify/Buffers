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

#include "ControlSystem.h"
#include "HvTable.h"

void cSystem_onMessage(HvBase *_c, void *o, int letIn, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const)) {

  HvMessage *n = HV_MESSAGE_ON_STACK(1);
  if (msg_compareSymbol(m, 0, "samplerate")) {
    msg_initWithFloat(n, msg_getTimestamp(m), (float) ctx_getSampleRate(_c));
  } else if (msg_compareSymbol(m, 0, "numInputChannels")) {
    msg_initWithFloat(n, msg_getTimestamp(m), (float) ctx_getNumInputChannels(_c));
  } else if (msg_compareSymbol(m, 0, "numOutputChannels")) {
    msg_initWithFloat(n, msg_getTimestamp(m), (float) ctx_getNumOutputChannels(_c));
  } else if (msg_compareSymbol(m, 0, "currentTime")) {
    msg_initWithFloat(n, msg_getTimestamp(m), (float) msg_getTimestamp(m));
  } else if (msg_compareSymbol(m, 0, "table")) {
    // NOTE(mhroth): no need to check message format for symbols as table lookup will fail otherwise
    HvTable *o = ctx_getTableForHash(_c, msg_getHash(m,1));
    if (o != NULL) {
      if (msg_compareSymbol(m, 2, "length")) {
        msg_initWithFloat(n, msg_getTimestamp(m), (float) hTable_getLength(o));
      } else if (msg_compareSymbol(m, 2, "size")) {
        msg_initWithFloat(n, msg_getTimestamp(m), (float) hTable_getSize(o));
      } else if (msg_compareSymbol(m, 2, "head")) {
        msg_initWithFloat(n, msg_getTimestamp(m), (float) hTable_getHead(o));
      } else return;
    } else return;
  } else return;
  sendMessage(_c, 0, n);
}
