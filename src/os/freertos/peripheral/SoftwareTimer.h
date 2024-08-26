/* Copyright 2024 teamprof.net@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <stdint.h>
// #include <Arduino.h>
// #include "../../../../../FreeRTOS-Kernel/include/timers.h"

#if defined ARDUPROF_FREERTOS

namespace ardufreertos
{
    class SoftwareTimer
    {
    public:
        // SoftwareTimer()
        // {
        //     hTimer = nullptr;
        // }

        SoftwareTimer(const char *pcTimerName,
                      const TickType_t xTimerPeriodInTicks,
                      const UBaseType_t uxAutoReload,
                      void *const pvTimerID,
                      TimerCallbackFunction_t pxCallbackFunction)
        {
            hTimer = xTimerCreate(pcTimerName,
                                  xTimerPeriodInTicks,
                                  uxAutoReload, // auto-reload when expire.
                                  pvTimerID,
                                  pxCallbackFunction);
        }

        ~SoftwareTimer()
        {
            xTimerDelete(hTimer, 0);
            hTimer = nullptr;
        }

        void start(void)
        {
            if (xTimerIsTimerActive(hTimer) == pdFALSE)
            {
                xTimerStart(hTimer, 0);
            }
        }

        void stop(void)
        {
            if (xTimerIsTimerActive(hTimer) != pdFALSE)
            {
                xTimerStop(hTimer, 0);
            }
        }

        TimerHandle_t timer(void)
        {
            return hTimer;
        }

        // virtual void onEventTimer(void) {}
        // // virtual void onEventTimer(void) = 0;

    protected:
        TimerHandle_t hTimer;

        // virtual void isr(TimerHandle_t xTimer) {}
        // virtual void IRAM_ATTR isr(TimerHandle_t xTimer) = 0;

    private:
    };

} // namespace ardufreertos

#endif // ARDUPROF_FREERTOS
