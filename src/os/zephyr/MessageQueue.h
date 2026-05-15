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

#ifdef ARDUPROF_ZEPHYR // #ifdef __ZEPHYR__

#include <stdint.h>
#include <zephyr/kernel.h>
#include "../../type/Message.h"

namespace zephyros
{
    class MessageQueue
    {
    public:
        MessageQueue(k_msgq *queue) : _queue(queue)
        {
        }
   
        void postEvent(k_msgq *queue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, k_timeout_t timeout = K_NO_WAIT)
        {
            Message msg = {
                .event = event,
                .iParam = iParam,
                .uParam = uParam,
                .lParam = lParam,
            };
            postEvent(queue, msg, timeout);
        }
        inline void postEvent(k_msgq *queue, const Message &msg, k_timeout_t timeout = K_NO_WAIT)
        {
            if (!queue)
            {
                return;
            }

            if (k_msgq_put(queue, &msg, timeout) != 0)
            {
                // LOG_INF("Failed to post event to app task event queue");
            }
        }

        inline void postEvent(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, k_timeout_t timeout = K_NO_WAIT)
        {
            postEvent(queue(), event, iParam, uParam, lParam, timeout);
        }
        inline void postEvent(const Message &msg, k_timeout_t timeout = K_NO_WAIT)
        {
            postEvent(queue(), msg, timeout);
        }

        inline k_msgq *queue(void)
        {
            return _queue;
        }

    protected:
        k_msgq *_queue;
    };
} // namespace zephyros

/////////////////////////////////////////////////////////////////////////////
#define __EVENT_MAP(class, event) \
    {                             \
        event, &class ::handler##event}
#define __EVENT_FUNC_DEFINITION(class, event, msg) void class ::handler##event(const Message &msg)
#define __EVENT_FUNC_DECLARATION(event) void handler##event(const Message &msg);
/////////////////////////////////////////////////////////////////////////////

#endif // ARDUPROF_ZEPHYR