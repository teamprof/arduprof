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
#include <mbed.h>
#include "../../type/Message.h"

using namespace std::chrono_literals;

#define DefaultQueueSize (128 * EVENTS_EVENT_SIZE)

///////////////////////////////////////////////////////////////////////////////
namespace ardumbedos
{
    class MessageQueue
    {
    public:
        using duration = std::chrono::duration<int, std::milli>;

        MessageQueue(uint16_t queueSize = DefaultQueueSize) : isStaticQueue(false),
                                                              _queue(new events::EventQueue(queueSize))
        {
        }

        MessageQueue(events::EventQueue *queue) : isStaticQueue(true),
                                                  _queue(queue)
        {
            assert(_queue);
        }

        ~MessageQueue()
        {
            if (!isStaticQueue && _queue)
            {
                delete _queue;
                _queue = nullptr;
            }
        }

        events::EventQueue *queue(void)
        {
            return _queue;
        }

        virtual void onMessage(const Message &msg)
        {
        }
        // virtual void onMessage(const Message &msg) = 0;

        void invokeOnMessage(const Message msg)
        {
            onMessage(msg);
        }

        void postEvent(MessageQueue *msgQueue, const Message &msg)
        {
            if (msgQueue && msgQueue->_queue)
            {
                auto ev = msgQueue->_queue->event(msgQueue, &MessageQueue::invokeOnMessage);
                ev.post(msg);
            }
        }
        void postEvent(MessageQueue *msgQueue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
        {
            Message msg = {
                .event = event,
                .iParam = iParam,
                .uParam = uParam,
                .lParam = lParam,
            };
            postEvent(msgQueue, msg);
        }

        inline void postEvent(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
        {
            postEvent(this, event, iParam, uParam, lParam);
        }

        inline void postEvent(const Message &message)
        {
            postEvent(this, message);
        }

    protected:
        events::EventQueue *_queue;
        bool isStaticQueue;
    };
} // ardumbedos

/////////////////////////////////////////////////////////////////////////////
#define __EVENT_MAP(class, event) \
    {                             \
        event, &class ::handler##event}
#define __EVENT_FUNC_DEFINITION(class, event, msg) void class ::handler##event(const Message &msg)
#define __EVENT_FUNC_DECLARATION(event) void handler##event(const Message &msg);
/////////////////////////////////////////////////////////////////////////////

#endif // ARDUPROF_MBED
