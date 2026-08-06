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

#if defined ARDUPROF_NUTTX
#include <pthread.h>
#include <sched.h>
// #include <debug.h>

#include "../MessageBus.h"

namespace nuttxos
{
    class ThreadBase : public MessageBus
    {
    public:
        ThreadBase(int core, size_t taskStackSize, int priority,
                   uint8_t *queueBuffer, 
                   size_t bufferSize,
                   void **queuePointers,
                   size_t lengthPointer) : MessageBus(queueBuffer, bufferSize, queuePointers, lengthPointer)
        // ThreadBase(int core, size_t taskStackSize, int priority,
        //            const char *queueName, long queueLength) : MessageBus(queueName, queueLength)
        {
#ifdef CONFIG_SMP
            CPU_ZERO(&_cpuset);
            if (core < 0 || core >= CONFIG_SMP_NCPUS)
            {
                // lowsyslog( "invalid param core = %d\n", core);
                syslog(LOG_ERR, "ThreadBase: invalid param core = %d\n", core);
                return;
            }
            CPU_SET(core, &_cpuset);
#endif

            auto err = pthread_attr_init(&_attr);
            if (err != OK)
            {
                // lowsyslog( "pthread_attr_init() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_attr_init() returns %d\n", err);
                return;
            }

            err = pthread_attr_setstacksize(&_attr, taskStackSize);
            if (err != OK)
            {
                // lowsyslog( "pthread_attr_setstacksize() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_attr_setstacksize() returns %d\n", err);
                return;
            }

            err = pthread_attr_setinheritsched(&_attr, PTHREAD_EXPLICIT_SCHED);
            if (err != OK)
            {
                // lowsyslog( "pthread_attr_setinheritsched() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_attr_setinheritsched() returns %d\n", err);
                return;
            }

            pthread_attr_setschedpolicy(&_attr, SCHED_FIFO);
            if (err != OK)
            {
                // lowsyslog( "pthread_attr_setschedpolicy() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_attr_setschedpolicy() returns %d\n", err);
                return;
            }

            _param.sched_priority = priority;
            pthread_attr_setschedparam(&_attr, &_param);
            if (err != OK)
            {
                // lowsyslog( "pthread_attr_setschedparam() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_attr_setschedparam() returns %d\n", err);
                return;
            }
        }

        virtual ~ThreadBase()
        {
            pthread_attr_destroy(&_attr);
        }

        virtual void start(void *context)
        {
            _context = context;
            auto err = pthread_create(&_thread, &_attr, [](void *arg) -> void *
                                      {
                                          // usleep(100 * 1000);
                                          // sleep(1);
                                          auto instance = reinterpret_cast<ThreadBase *>(arg);
                                          instance->run();
                                          return nullptr;
                                          //
                                      },
                                      this);
            if (err != OK)
            {
                // lowsyslog("pthread_create() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_create() returns %d\n", err);
                return;
            }

#ifdef CONFIG_SMP
            err = pthread_setaffinity_np(_thread, sizeof(cpu_set_t), &_cpuset);
            if (err != OK)
            {
                // lowsyslog( "pthread_setaffinity_np() returns %d\n", err);
                syslog(LOG_ERR, "ThreadBase: pthread_setaffinity_np() returns %d\n", err);
                return;
            }
#endif

            // syslog(LOG_DEBUG, "start() success\n");
        }

        virtual void run(void)
        {
            setup();
            // __ASSERT(queue());
            messageLoopForever();
        }

        pthread_t *getThreadPtr(void)
        {
            return &_thread;
        }

    protected:
        virtual void setup(void) {}

        pthread_t _thread;
        pthread_attr_t _attr;
        struct sched_param _param;

#ifdef CONFIG_SMP
        cpu_set_t _cpuset;
#endif
    };
} // namespace nuttxos
#endif // ARDUPROF_NUTTX