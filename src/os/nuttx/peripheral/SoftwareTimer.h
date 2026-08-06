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

#if defined ARDUPROF_NUTTX

#include <time.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <nuttx/wqueue.h>
// #include <mqueue.h>
// #include <sys/types.h>

namespace nuttxos
{
    class SoftwareTimer
    {
    public:
        typedef void (*Callback)(timer_t timer_id);

        SoftwareTimer(const struct itimerspec &its,
                      Callback callback = nullptr) : _timerid(nullptr),
                                                     _its(its),
                                                     _callback(callback),
                                                     _isStarted(false)
        {
            timer_t timerid;
            struct sigevent sev;
            memset(&sev, 0, sizeof(struct sigevent));
            sev.sigev_notify = SIGEV_THREAD;
            sev.sigev_notify_function = [](union sigval val)
            {
                auto instance = reinterpret_cast<SoftwareTimer *>(val.sival_ptr);
                if (work_available(&instance->_timer_work))
                {
                    // Immediately schedule the work to execute on the HP Work Queue
                    work_queue(USRWORK, &instance->_timer_work, [](void *arg)
                               // work_queue(HPWORK, &instance->_timer_work, [](void *arg)
                               {
                                   auto inst = reinterpret_cast<SoftwareTimer *>(arg);
                                   if (inst->_callback)
                                   {
                                       (*inst->_callback)(inst->timer());
                                   }
                                   //
                               },
                               instance, 0);
                }

                // if (instance->_callback)
                // {
                //     (*instance->_callback)(instance->timer());
                // }
                //
            };
            sev.sigev_value.sival_ptr = this;
            if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == ERROR)
            {
                if (errno != EAGAIN)
                {
                    syslog(LOG_ERR, "Timer: timer_create() returns %d\n", errno);
                }
                return;
            }

            _timerid = timerid;
            syslog(LOG_DEBUG, "SoftwareTimer: timer_create() success\n");
        }

        ~SoftwareTimer()
        {
            if (_timerid)
            {
                stop();
                if (timer_delete(_timerid) == ERROR)
                {
                    syslog(LOG_ERR, "SoftwareTimer: timer_delete() returns %d\n", errno);
                }
                else
                {
                    syslog(LOG_DEBUG, "~SoftwareTimer: success\n");
                }
                _timerid = nullptr;
            }
        }

        virtual void start(void)
        {
            if (_timerid && !_isStarted)
            {
                if (timer_settime(_timerid, 0, &_its, NULL) == ERROR)
                {
                    syslog(LOG_ERR, "SoftwareTimer: timer_settime() returns %d\n", errno);
                    return;
                }

                syslog(LOG_DEBUG, "SoftwareTimer: timer_settime() success\n");

                _isStarted = true;
            }
        }

        void stop(void)
        {
            if (_timerid && _isStarted)
            {
                _isStarted = false;

                struct itimerspec its;
                memset(&its, 0, sizeof(its));
                if (timer_settime(_timerid, 0, &its, nullptr) == ERROR)
                {
                    syslog(LOG_ERR, "SoftwareTimer: timer_settime() returns %d\n", errno);
                }
                else
                {
                    syslog(LOG_DEBUG, "SoftwareTimer: stop success\n");
                }
            }
        }

        timer_t timer(void)
        {
            return _timerid;
        }

    protected:
        timer_t _timerid;
        const struct itimerspec _its;
        Callback _callback;
        bool _isStarted;
        struct work_s _timer_work;

    private:
    };

} // namespace nuttxos

static_assert(sizeof(time_t) != sizeof(uint32_t), "Critical Error: sizeof(time_t) MUST be equal to sizeof(uint32_t)");

#endif // ARDUPROF_NUTTX
