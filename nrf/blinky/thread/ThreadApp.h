/* Copyright 2026 teamprof.net@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include "../AppEvent.h"
#include "../ArduProfApp.h"
#include <map>
#include <stdbool.h>
#include <stdint.h>

#undef CLASSNAME
#define CLASSNAME ThreadApp

class CLASSNAME : public zephyros::ThreadBase {
  public:
    static CLASSNAME *getInstance(void);
    virtual void start(void *);
    virtual void onMessage(const Message &msg);
    virtual void setup(void);

  protected:
    typedef void (CLASSNAME::*handlerFunc)(const Message &);
    std::map<int16_t, handlerFunc> _handlerMap;

  private:
    CLASSNAME();
    static CLASSNAME *_instance;
    static struct k_timer _timer1Hz;

    static void _timerExpiryHandler(struct k_timer *timer_id);
    static void _timerStopHandler(struct k_timer *timer);
    void handlerSoftwareTimer(k_timer *timer);

    ///////////////////////////////////////////////////////////////////////////
    // event handler
    ///////////////////////////////////////////////////////////////////////////
    __EVENT_FUNC_DECLARATION(EventSystem)

    // void handlerEventNull(const Message &msg);
    __EVENT_FUNC_DECLARATION(EventNull)
};
