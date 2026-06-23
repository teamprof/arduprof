/* Copyright 2026 teamprof.net@gmail.com
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

#if defined ARDUPROF_ZEPHYR

#include <zephyr/kernel.h>

namespace zephyros
{
    class SoftwareTimer
    {
    public:
        typedef void (*Callback)(struct k_timer *timer, bool isStop);

        SoftwareTimer(k_timeout_t duration,
                      k_timeout_t period,
                      Callback callback = nullptr) : _duration(duration), _period(period), _callback(callback)
        {
            k_timer_init(&_timer, [](struct k_timer *timer)
                         {
                             auto instance = CONTAINER_OF(timer, SoftwareTimer, _timer);
                             if (instance->_callback)
                             {
                                 instance->_callback(timer, false);
                             }
                             //
                         },
                         [](struct k_timer *timer)
                         {
                             auto instance = CONTAINER_OF(timer, SoftwareTimer, _timer);
                             if (instance->_callback)
                             {
                                 instance->_callback(timer, true);
                             }
                             //
                         });
        }

        ~SoftwareTimer()
        {
            stop();
        }

        void start(void)
        {
            k_timer_start(&_timer, _duration, _period);
        }

        void stop(void)
        {
            k_timer_stop(&_timer);
        }

        struct k_timer *timer(void)
        {
            return &_timer;
        }

    protected:
        struct k_timer _timer;
        k_timeout_t _duration;
        k_timeout_t _period;
        Callback _callback;

    private:
    };

} // namespace zephyros

#endif // ARDUPROF_ZEPHYR
