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
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>

#include "./ThreadApp.h"

#define LOG_LEVEL 4
LOG_MODULE_REGISTER(CLASSNAME, LOG_LEVEL);

///////////////////////////////////////////////////////////////////////
#define TASK_STACK_SIZE 1024
#define TASK_PRIORITY 8    // Lower Number = Higher Priority, number is 0 ~ 15
#define TASK_OPTIONS 0     // e.g. K_INHERIT_PERMS | K_NO_SLEEP
#define TASK_QUEUE_SIZE 16 // message queue size for app task
static_assert(TASK_PRIORITY <= K_LOWEST_APPLICATION_THREAD_PRIO,
              "TASK_PRIORITY exceeds K_LOWEST_APPLICATION_THREAD_PRIO");

#define QUEUENAME CONCAT(CLASSNAME, taskQueue)

///////////////////////////////////////////////////////////////////////
void CLASSNAME::_timerExpiryHandler(struct k_timer *timer_id)
{
    auto instance = CLASSNAME::getInstance();
    instance->postEvent(EventSystem, SysSoftwareTimer, 0, (uint32_t)timer_id);
}
void CLASSNAME::_timerStopHandler(struct k_timer *timer)
{
    LOG_DBG("timer=%p", timer);
}

K_TIMER_DEFINE(CLASSNAME::_timer1Hz, CLASSNAME::_timerExpiryHandler,
               CLASSNAME::_timerStopHandler);

///////////////////////////////////////////////////////////////////////
static K_THREAD_STACK_DEFINE(_task_stack, TASK_STACK_SIZE);
K_MSGQ_DEFINE(QUEUENAME, sizeof(Message), TASK_QUEUE_SIZE, alignof(uint32_t));

CLASSNAME *CLASSNAME::_instance = NULL;

///////////////////////////////////////////////////////////////////////
CLASSNAME::CLASSNAME() : zephyros::ThreadBase(&QUEUENAME)
{
    _handlerMap = {
        __EVENT_MAP(CLASSNAME, EventSystem),

        // {EventNull, &ThreadSlave::handlerEventNull},
        __EVENT_MAP(CLASSNAME, EventNull),
    };
}

CLASSNAME *CLASSNAME::getInstance(void)
{
    if (!_instance)
    {
        static CLASSNAME instance;
        _instance = &instance;
    }
    return _instance;
}

void CLASSNAME::start(void *ctx)
{
    ThreadBase::start(ctx);

    _tid = k_thread_create(
        &_task_data, _task_stack, K_THREAD_STACK_SIZEOF(_task_stack),
        [](void *p1, void *p2, void *p3)
        {
            static_cast<CLASSNAME *>(p1)->run();
        },
        this, NULL, NULL, TASK_PRIORITY, TASK_OPTIONS, K_NO_WAIT);
}

void CLASSNAME::setup(void)
{
    k_timer_start(&_timer1Hz, K_MSEC(1000), K_SECONDS(1));
}

void CLASSNAME::onMessage(const Message &msg)
{
    auto func = _handlerMap[msg.event];
    if (func)
    {
        (this->*func)(msg);
    }
    else
    {
        LOG_DBG("Unsupported event=%hd, iParam=%hd, uParam=%hu, lParam=%u",
                msg.event, msg.iParam, msg.uParam, msg.lParam);
    }
}

///////////////////////////////////////////////////////////////////////
// void ThreadSlave::handlerEventSystem(const Message &msg)
__EVENT_FUNC_DEFINITION(CLASSNAME, EventSystem, msg)
{
    // LOG_DBG("EventSystem(%hd), iParam=%hd, uParam=%hu, lParam=0x%08x",
    //         msg.event, msg.iParam, msg.uParam, msg.lParam);
    enum SystemTriggerSource src =
        static_cast<enum SystemTriggerSource>(msg.iParam);
    switch (src)
    {
    case SysSoftwareTimer:
        handlerSoftwareTimer((k_timer *)(msg.lParam));
        break;
    default:
        LOG_WRN("unsupported SystemTriggerSource=%hd", src);
        break;
    }
}
// void ThreadSlave::handlerEventNull(const Message &msg)
__EVENT_FUNC_DEFINITION(CLASSNAME, EventNull, msg)
{
    LOG_DBG("EventNull(%hd), iParam=%hd, uParam=%hu, lParam=%u", msg.event,
            msg.iParam, msg.uParam, msg.lParam);
}

///////////////////////////////////////////////////////////////////////
void CLASSNAME::handlerSoftwareTimer(k_timer *timer)
{
    if (timer == &_timer1Hz)
    {
        LOG_DBG("timer1Hz");
    }
    else
    {
        LOG_DBG("unsupported timer=%p", timer);
    }
}
