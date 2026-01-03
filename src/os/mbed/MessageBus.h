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

#if defined ARDUPROF_MBED
#include <Arduino.h>
#include "./MessageQueue.h"

namespace ardumbedos
{

    class MessageBus : public MessageQueue
    {
    public:
        MessageBus(events::EventQueue *queue) : MessageQueue(queue), _context(nullptr), _isDone(false)
        {
        }

        MessageBus(uint16_t queueSize = DefaultQueueSize) : MessageQueue(queueSize), _context(nullptr), _isDone(false)
        {
        }

        virtual void start(void *context)
        {
            _context = context;
        }

        virtual void onMessage(const Message &msg) = 0;

        virtual void messageLoop(int ms = -1)
        {
            assert(_queue);

            // modification to replace depreciated api dispatch()
            std::chrono::milliseconds duration_ms(ms);
            _queue->dispatch_for(duration_ms);
            // _queue->dispatch(ms);
        }

        virtual void messageLoopForever(void)
        {
            while (!_isDone)
            {
                messageLoop();
            }
        }

        void *context(void)
        {
            return _context;
        }

    protected:
        void *_context;

        bool _isDone;
    };
} // ardumbedos

#endif // ARDUPROF_MBED