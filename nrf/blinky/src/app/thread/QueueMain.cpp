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
#include <stdbool.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

#include "./QueueMain.h"

#define LOG_LEVEL 4
LOG_MODULE_REGISTER(QueueMain, LOG_LEVEL);

///////////////////////////////////////////////////////////////////////
#define USER_LED DK_LED1

///////////////////////////////////////////////////////////////////////
void timerExpiryHandler(struct k_timer *timer_id)
{
    auto instance = QueueMain::getInstance();
    instance->postEvent(EventSystem, SysSoftwareTimer, 0, (uint32_t)timer_id);
}
void timerStopHandler(struct k_timer *timer)
{
    LOG_DBG("timer=%p", timer);
}

K_TIMER_DEFINE(timer1Hz, timerExpiryHandler, timerStopHandler);

///////////////////////////////////////////////////////////////////////
#define TASK_QUEUE_SIZE 32 // message queue size for app task
K_MSGQ_DEFINE(taskQueue, sizeof(Message), TASK_QUEUE_SIZE, alignof(uint32_t));

QueueMain *QueueMain::_instance = NULL;

///////////////////////////////////////////////////////////////////////
QueueMain::QueueMain() : zephyros::MessageBus(&taskQueue), _ledState(false)
{
    handlerMap = {
        __EVENT_MAP(QueueMain, EventSystem),
        __EVENT_MAP(QueueMain, EventNull), // {EventNull, &QueueMain::handlerEventNull},
    };
}

QueueMain *QueueMain::getInstance(void)
{
    if (!_instance)
    {
        static QueueMain mainTask;
        _instance = &mainTask;
    }
    return _instance;
}

void QueueMain::start(void *ctx)
{
    MessageBus::start(ctx);

    int err = dk_leds_init();
    if (err)
    {
        LOG_ERR("dk_leds_init() returns %d", err);
    }

    dk_set_led(USER_LED, _ledState);
    // dk_set_led_on(USER_LED);
    // dk_set_led_off(USER_LED);

    k_timer_start(&timer1Hz, K_MSEC(1000), K_SECONDS(1));
}

void QueueMain::onMessage(const Message &msg)
{
    auto func = handlerMap[msg.event];
    if (func)
    {
        (this->*func)(msg);
    }
    else
    {
        LOG_DBG("Unsupported event=%hd, iParam=%hd, uParam=%hu, lParam=%u", msg.event, msg.iParam, msg.uParam, msg.lParam);
    }
}

///////////////////////////////////////////////////////////////////////
bool QueueMain::getLedState(void)
{
    return _ledState;
}
void QueueMain::setLedState(bool ledState)
{
    _ledState = ledState;
    LOG_DBG("_ledState=%d", _ledState);
    dk_set_led(USER_LED, ledState);
}

void QueueMain::toggleLedState(void)
{
    setLedState(!_ledState);
}

///////////////////////////////////////////////////////////////////////

__EVENT_FUNC_DEFINITION(QueueMain, EventSystem, msg) // void QueueMain::handlerEventSystem(const Message &msg)
{
    // LOG_DBG("EventSystem(%hd), iParam=%hd, uParam=%hu, lParam=0x%08x", msg.event, msg.iParam, msg.uParam, msg.lParam);
    enum SystemTriggerSource src = static_cast<enum SystemTriggerSource>(msg.iParam);
    switch (src)
    {
    case SysSoftwareTimer:
        handlerSoftwareTimer((k_timer *)(msg.lParam));
        break;
    default:
        LOG_DBG("unsupported SystemTriggerSource=%hd", src);
        break;
    }
}
__EVENT_FUNC_DEFINITION(QueueMain, EventNull, msg) // void QueueMain::handlerEventNull(const Message &msg)
{
    LOG_DBG("EventNull(%hd), iParam=%hd, uParam=%hu, lParam=%u", msg.event, msg.iParam, msg.uParam, msg.lParam);
}

void QueueMain::handlerSoftwareTimer(k_timer *timer)
{
    if (timer == &timer1Hz)
    {
        // LOG_DBG("timer1Hz");
        toggleLedState();
    }
    else
    {
        LOG_DBG("unsupported timer=%p", timer);
    }
}
