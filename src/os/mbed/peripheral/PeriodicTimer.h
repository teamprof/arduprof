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

#if defined ARDUPROF_MBED

// #include <stdint.h>
#include "./SoftwareTimer.h"

namespace ardumbedos
{
    class PeriodicTimer : public SoftwareTimer
    {
    public:
        PeriodicTimer(events::EventQueue *queue, duration ms, Callback callback) : SoftwareTimer(queue, ms, callback)
        {
        }

        virtual void start(void)
        {
            if (_queue)
            {
                _id = _queue->call_every(_ms, [this]()
                                         {
                                             if (_callback)
                                             {
                                                 _callback(_id);
                                             } });
            }
        }

    private:
    };

} // namespace ardumbedos

#endif // ARDUPROF_MBED
