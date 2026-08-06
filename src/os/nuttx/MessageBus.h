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

#ifdef ARDUPROF_NUTTX

#include <stdbool.h>
#include <errno.h>
#include <syslog.h>

#include "./MessageQueue.h"

namespace nuttxos
{
    class MessageBus : public MessageQueue
    {
    public:
        MessageBus(uint8_t *queueBuffer, 
                   size_t bufferSize,
                   void **queuePointers,
                   size_t lengthPointer) : MessageQueue(queueBuffer, bufferSize, queuePointers, lengthPointer),
                                           _context(nullptr),
                                           _isDone(false)
        {
        }

        // MessageBus(const char *queueName, long length) : MessageQueue(queueName, length),
        //                                                  _context(nullptr),
        //                                                  _isDone(false)
        // {
        // }

        virtual void start(void *context)
        {
            _context = context;
        }

        virtual void onMessage(const Message &msg) = 0;

        virtual void messageLoop(uint32_t delay = -1)
        {
            if(delay == (uint32_t)(-1)) {
                auto err = nxsem_wait(&_sem_queue);
                if (err < 0) {
                    syslog(LOG_ERR, "messageLoop: nxsem_wait() failed with %d\n", err);
                    return;
                }
            } else {
                auto err = nxsem_tickwait(&_sem_queue, delay);
                if (err < 0) {
                    if(err != -ETIMEDOUT)
                    {
                        syslog(LOG_ERR, "messageLoop: nxsem_tickwait() failed with %d\n", err);
                    }
                    return;
                }
            }

            Message *ptr = NULL;
            irqstate_t flags = spin_lock_irqsave(&_lock_pool);
            if (_queue_tail != _queue_head) 
            {
                ptr = (Message *)_queue_pointers[_queue_tail];
                _queue_tail = (_queue_tail + 1) % _length_pointers;
            }
            // int queue_head = _queue_head, queue_tail = _queue_tail;
            spin_unlock_irqrestore(&_lock_pool, flags);

            // syslog(LOG_DEBUG, "MessageBus: messageLoop: _queue_head=%d, _queue_tail=%d\n", queue_head, queue_tail);

            if (ptr != NULL)
            {
                onMessage(*ptr);

                // Return memory block to Granule Pool 
                gran_free(_msg_pool, ptr, SIZEOF_MESSAGE_POW2);
            }
        }
        // virtual void messageLoop(const struct timespec *timeout = nullptr)
        // {
        //     ssize_t bytes_read;
        //     Message msg;
        //     unsigned int prio;

        //     if (!timeout)
        //     {
        //         bytes_read = mq_receive(*queue_rd(), (char *)&msg, sizeof(msg), &prio);
        //     }
        //     else
        //     {
        //         struct timespec ts;
        //         if (clock_gettime(CLOCK_REALTIME, &ts) != OK)
        //         {
        //             syslog(LOG_ERR, "messageLoop: clock_gettime() failed\n");
        //             return;
        //         }

        //         ts.tv_sec += timeout->tv_sec;
        //         ts.tv_nsec += timeout->tv_nsec;
        //         bytes_read = mq_timedreceive(*queue_rd(), (char *)&msg, sizeof(msg), &prio, &ts);
        //     }

        //     if (bytes_read >= 0)
        //     {
        //         // syslog(LOG_DEBUG, "bytes_read=%u\n", bytes_read);
        //         onMessage(msg);
        //     }
        //     else if (errno != ETIMEDOUT)
        //     {
        //         syslog(LOG_WARNING, "messageLoop: errno=%d\n", errno);
        //     }
        // }

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

} // namespace nuttxos

#endif // ARDUPROF_NUTTX