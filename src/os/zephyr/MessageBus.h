/* Copyright 2023 teamprof.net@gmail.com
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

#ifdef ARDUPROF_ZEPHYR

#include <stdbool.h>
#include "./MessageQueue.h"

namespace zephyros
{
    class MessageBus : public MessageQueue
    {
    public:
        MessageBus(k_msgq *queue) : MessageQueue(queue), _context(nullptr), _isDone(false)
        {
        }

        virtual void start(void *context)
        {
            _context = context;
        }

        virtual void onMessage(const Message &msg) = 0;

        virtual void messageLoop(k_timeout_t timeout = K_FOREVER)
        {
            Message msg;
            if (!k_msgq_get(queue(), &msg, timeout))
            {
                onMessage(msg);
            }
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

} // namespace zephyros

#endif // ARDUPROF_ZEPHYR