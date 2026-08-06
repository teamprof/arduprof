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

#include <stdint.h>
#include <mqueue.h>
#include <errno.h>
#include <syslog.h>
// #include <nuttx/mqueue.h>
#include <nuttx/spinlock.h>
#include <nuttx/semaphore.h>
#include <nuttx/mm/gran.h>

#include "../../type/Message.h"

#define NEXT_POW2_PAD1(x)  ((x) | ((x) >> 1))
#define NEXT_POW2_PAD2(x)  (NEXT_POW2_PAD1(x) | (NEXT_POW2_PAD1(x) >> 2))
#define NEXT_POW2_PAD4(x)  (NEXT_POW2_PAD2(x) | (NEXT_POW2_PAD2(x) >> 4))
#define NEXT_POW2_PAD8(x)  (NEXT_POW2_PAD4(x) | (NEXT_POW2_PAD4(x) >> 8))
#define NEXT_POW2_PAD16(x) (NEXT_POW2_PAD8(x) | (NEXT_POW2_PAD8(x) >> 16))

// Takes size `s`, subtracts 1, fills bits right, adds 1 
#define ROUNDUP_POW2(s) \
    ((s) == 0 ? 1 : (NEXT_POW2_PAD16((size_t)(s) - 1) + 1))

#define SIZEOF_MESSAGE_POW2 ROUNDUP_POW2(sizeof(Message))



#define LOG2_POW2_32(x) ( \
    (x) >= (1U << 31) ? 31 : (x) >= (1U << 30) ? 30 : \
    (x) >= (1U << 29) ? 29 : (x) >= (1U << 28) ? 28 : \
    (x) >= (1U << 27) ? 27 : (x) >= (1U << 26) ? 26 : \
    (x) >= (1U << 25) ? 25 : (x) >= (1U << 24) ? 24 : \
    (x) >= (1U << 23) ? 23 : (x) >= (1U << 22) ? 22 : \
    (x) >= (1U << 21) ? 21 : (x) >= (1U << 20) ? 20 : \
    (x) >= (1U << 19) ? 19 : (x) >= (1U << 18) ? 18 : \
    (x) >= (1U << 17) ? 17 : (x) >= (1U << 16) ? 16 : \
    (x) >= (1U << 15) ? 15 : (x) >= (1U << 14) ? 14 : \
    (x) >= (1U << 13) ? 13 : (x) >= (1U << 12) ? 12 : \
    (x) >= (1U << 11) ? 11 : (x) >= (1U << 10) ? 10 : \
    (x) >= (1U << 9)  ?  9 : (x) >= (1U << 8)  ?  8 : \
    (x) >= (1U << 7)  ?  7 : (x) >= (1U << 6)  ?  6 : \
    (x) >= (1U << 5)  ?  5 : (x) >= (1U << 4)  ?  4 : \
    (x) >= (1U << 3)  ?  3 : (x) >= (1U << 2)  ?  2 : \
    (x) >= (1U << 1)  ?  1 : 0)

#define LOG2_MESSAGE_SIZE LOG2_POW2_32(SIZEOF_MESSAGE_POW2)




namespace nuttxos
{
    class MessageQueue
    {
    public:
        MessageQueue(uint8_t *queueBuffer, 
                     size_t bufferSize,
                     void **queuePointers,
                     size_t lengthPointers) : _lock_pool(SP_UNLOCKED),
                                              _queue_pointers(queuePointers), 
                                              _length_pointers(lengthPointers), 
                                              _queue_head(0), 
                                              _queue_tail(0)
        {
            nxsem_init(&_sem_queue, 0, 0);
            _msg_pool = gran_initialize(queueBuffer, bufferSize, LOG2_MESSAGE_SIZE, 2);
        }


        // MessageQueue(const char *queueName, long length)
        // {
        //     struct mq_attr attr = {0};
        //     attr.mq_flags = 0;
        //     attr.mq_maxmsg = length;
        //     attr.mq_msgsize = sizeof(Message);
        //     attr.mq_curmsgs = 0;

        //     auto mqfd_wr = mq_open(queueName, O_RDWR | O_CREAT | O_NONBLOCK, 0666, &attr);
        //     // auto mqfd = mq_open(queueName, O_RDWR | O_CREAT, 0666, &attr);
        //     if (mqfd_wr == ERROR)
        //     {
        //         _mqfd_rd = ERROR;
        //         _mqfd_wr = ERROR;
        //         // _queue = nullptr;
        //         syslog(LOG_ERR, "MessageQueue: mq_open(%s) ERROR: mqfd_wr\n", queueName);
        //         return;
        //     }

        //     auto mqfd_rd = mq_open(queueName, O_RDONLY, 0666, &attr);
        //     if (mqfd_rd == ERROR)
        //     {
        //         if (mq_close(mqfd_wr) == ERROR)
        //         {
        //             syslog(LOG_ERR, "MessageQueue: mq_close(mqfd_wr) ERROR\n");
        //         }

        //         _mqfd_rd = ERROR;
        //         _mqfd_wr = ERROR;
        //         // _queue = nullptr;
        //         syslog(LOG_ERR, "MessageQueue: mq_open(%s) ERROR: mqfd_rd\n", queueName);
        //         return;
        //     }

        //     _mqfd_rd = mqfd_rd;
        //     _mqfd_wr = mqfd_wr;
        //     syslog(LOG_DEBUG, "MessageQueue: mq_open(%s) success: _mqfd_wr=%d, _mqfd_rd=%d", queueName, _mqfd_wr, _mqfd_rd);
        // }

        inline void postEvent(void *msg_pool, const Message &msg, int prio = MQ_PRIO_MAX)
        {
            if(!msg_pool) 
            {
                return;
            }

            auto *ptr = (Message *)gran_alloc(msg_pool, SIZEOF_MESSAGE_POW2);
            if (!ptr)
            {
                return;
            }
            memcpy(ptr, &msg, sizeof(msg));

            irqstate_t flags = spin_lock_irqsave(&_lock_pool);
            int next_head = (_queue_head + 1) % _length_pointers;
            // int queue_tail = _queue_tail;
            if (next_head != _queue_tail) // Ensure FIFO isn't full
            {
                _queue_pointers[_queue_head] = ptr;
                _queue_head = next_head;

                // Release spinlock BEFORE signaling semaphore 
                spin_unlock_irqrestore(&_lock_pool, flags);

                // Signal Consumer Thread (nxsem_post is safe in ISRs & threads) 
                nxsem_post(&_sem_queue);
            }
            else
            {
                // FIFO full! Release spinlock first, then free the allocated memory.
                // gran_free() is also internally thread-safe with CONFIG_GRAN_INTR=y.
                spin_unlock_irqrestore(&_lock_pool, flags);
                gran_free(msg_pool, ptr, SIZEOF_MESSAGE_POW2);
            }
            
            // syslog(LOG_DEBUG, "MessageQueue: postEvent: _queue_head=%d, _queue_tail=%d\n", next_head, queue_tail);
        }
        // inline void postEvent(mqd_t *queue, const Message &msg, int prio = MQ_PRIO_MAX)
        // {
        //     if (!queue)
        //     {
        //         return;
        //     }

        //     // struct timespec ts;
        //     // ts.tv_sec = 0;
        //     // ts.tv_nsec = 0;
        //     // auto ret = mq_timedsend(*queue, (const char *)&msg, sizeof(msg), prio, &ts);

        //     // O_NONBLOCK ensure immediately return when queue is full
        //     // auto ret = nxmq_send(*queue, (const char *)&msg, sizeof(msg), prio);
        //     auto ret = mq_send(*queue, (const char *)&msg, sizeof(msg), prio);
        //     if (ret != OK)
        //     {
        //         syslog(LOG_WARNING, "MessageQueue: mq_send(%d) errno=%d\n", *queue, errno);
        //         // syslog(LOG_WARNING, "MessageQueue: mq_send() errno=%d\n", errno);
        //     }
        // }

        void postEvent(void *msg_pool, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, int prio = MQ_PRIO_MAX)
        {
            Message msg = {
                .event = event,
                .iParam = iParam,
                .uParam = uParam,
                .lParam = lParam,
            };

            postEvent(msg_pool, msg, prio);
        }        

        // void postEvent(mqd_t *queue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, int prio = MQ_PRIO_MAX)
        // {
        //     Message msg = {
        //         .event = event,
        //         .iParam = iParam,
        //         .uParam = uParam,
        //         .lParam = lParam,
        //     };

        //     postEvent(queue, msg, prio);
        // }        

        inline void postEvent(MessageQueue *msgQueue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, int prio = MQ_PRIO_MAX)
        {
            if (!msgQueue || !msgQueue->_msg_pool)
            {
                return;
            }
            postEvent(msgQueue->_msg_pool, event, iParam, uParam, lParam, prio);
        }
        // inline void postEvent(MessageQueue *msgQueue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, int prio = MQ_PRIO_MAX)
        // {
        //     if (!msgQueue || !msgQueue->queue_wr())
        //     {
        //         return;
        //     }
        //     postEvent(msgQueue->queue_wr(), event, iParam, uParam, lParam, prio);
        // }

        inline void postEvent(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, int prio = MQ_PRIO_MAX)
        {
            postEvent(_msg_pool, event, iParam, uParam, lParam, prio);
            // postEvent(queue_wr(), event, iParam, uParam, lParam, prio);
        }
        inline void postEvent(const Message &msg, int prio = MQ_PRIO_MAX)
        {
            postEvent(_msg_pool, msg, prio);
            // postEvent(queue_wr(), msg, prio);
        }

        // inline mqd_t *queue_wr(void)
        // {
        //     return &_mqfd_wr;
        // }
        // inline mqd_t *queue_rd(void)
        // {
        //     return &_mqfd_rd;
        // }


    protected:
        sem_t _sem_queue;
        spinlock_t _lock_pool;
        void *_msg_pool;
        void **_queue_pointers;
        const size_t _length_pointers;
        volatile int _queue_head;
        volatile int _queue_tail;

        // mqd_t *_queue;
        // mqd_t _mqfd_rd;
        // mqd_t _mqfd_wr;

    private:
    };

} // namespace nuttxos

/////////////////////////////////////////////////////////////////////////////
#define __EVENT_MAP(class, event) \
    {                             \
        event, &class ::handler##event}
#define __EVENT_FUNC_DEFINITION(class, event, msg) void class ::handler##event(const Message &msg)
#define __EVENT_FUNC_DECLARATION(event) void handler##event(const Message &msg);
/////////////////////////////////////////////////////////////////////////////

#endif // ARDUPROF_NUTTX