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
#include "./ThreadApp.h"
#include "./AppContext.h"

////////////////////////////////////////////////////////////////////////////////////////////
ThreadApp *ThreadApp::_instance = nullptr;

#if defined ARDUPROF_FREERTOS && defined ARDUINO_ARCH_RP2040
////////////////////////////////////////////////////////////////////////////////////////////
// Thread for FreeRTOS RP2040/RP2350
////////////////////////////////////////////////////////////////////////////////////////////

static constexpr UBaseType_t uxCoreAffinityMask = ((1 << 0)); // task only run on core 0
// static constexpr UBaseType_t uxCoreAffinityMask = ((1 << 1)); // task only run on core 1
// static constexpr uxCoreAffinityMask = ( ( 1 << 0 ) | ( 1 << 2 ) );  // e.g. task can only run on core 0 and core 2

#define TASK_NAME "ThreadApp"
#define TASK_STACK_SIZE (4096 / sizeof(StackType_t))
#define TASK_PRIORITY 10  // Priority, (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
#define TASK_QUEUE_SIZE 8 // message queue size for app task

#define TASK_INIT_NAME "taskDelayInit"
#define TASK_INIT_STACK_SIZE (4096 / sizeof(StackType_t))
#define TASK_INIT_PRIORITY 0

static uint8_t ucQueueStorageArea[TASK_QUEUE_SIZE * sizeof(Message)];
static StaticQueue_t xStaticQueue;

static StackType_t xStack[TASK_STACK_SIZE];
static StaticTask_t xTaskBuffer;

///////////////////////////////////////////////////////////////////////
ThreadApp::ThreadApp() : ThreadBase(TASK_QUEUE_SIZE, ucQueueStorageArea, &xStaticQueue),
                         _handlerMap()
{
    _instance = this;

    _handlerMap = {
        __EVENT_MAP(ThreadApp, EventNull), // {EventNull, &ThreadApp::handlerEventNull},
    };
}

void ThreadApp::start(void *ctx)
{
    LOG_TRACE("core", get_core_num());
    configASSERT(ctx);
    _context = ctx;

    _taskHandle = xTaskCreateStatic(
        [](void *instance)
        { static_cast<ThreadBase *>(instance)->run(); },
        TASK_NAME,
        TASK_STACK_SIZE, // This stack size can be checked & adjusted by reading the Stack Highwater
        this,
        TASK_PRIORITY, // Priority, (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        xStack,
        &xTaskBuffer);
    vTaskCoreAffinitySet(_taskHandle, uxCoreAffinityMask); // Set the core affinity mask for the task, i.e. set task on running core
}

#elif defined ARDUPROF_FREERTOS && defined ESP_PLATFORM
////////////////////////////////////////////////////////////////////////////////////////////
// Thread for ESP32
////////////////////////////////////////////////////////////////////////////////////////////

// #define RUNNING_CORE 0 // dedicate core 0 for Thread
// #define RUNNING_CORE 1 // dedicate core 1 for Thread
#define RUNNING_CORE ARDUINO_RUNNING_CORE

#define TASK_NAME "ThreadApp"
#define TASK_STACK_SIZE (4096 / sizeof(StackType_t))
#define TASK_PRIORITY 3
#define TASK_QUEUE_SIZE 8 // message queue size for app task

#define TASK_INIT_NAME "taskDelayInit"
#define TASK_INIT_STACK_SIZE (4096 / sizeof(StackType_t))
#define TASK_INIT_PRIORITY 0

static uint8_t ucQueueStorageArea[TASK_QUEUE_SIZE * sizeof(Message)];
static StaticQueue_t xStaticQueue;

static StackType_t xStack[TASK_STACK_SIZE];
static StaticTask_t xTaskBuffer;

////////////////////////////////////////////////////////////////////////////////////////////
ThreadApp::ThreadApp() : ardufreertos::ThreadBase(TASK_QUEUE_SIZE, ucQueueStorageArea, &xStaticQueue),
                         _handlerMap()
{
    _instance = this;

    // setup event handlers
    _handlerMap = {
        __EVENT_MAP(ThreadApp, EventNull), // {EventNull, &ThreadApp::handlerEventNull},
    };
}

void ThreadApp::start(void *ctx)
{
    // LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    ThreadBase::start(ctx);

    _taskHandle = xTaskCreateStaticPinnedToCore(
        [](void *instance)
        { static_cast<ThreadBase *>(instance)->run(); },
        TASK_NAME,
        TASK_STACK_SIZE, // This stack size can be checked & adjusted by reading the Stack Highwater
        this,
        TASK_PRIORITY, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        xStack,
        &xTaskBuffer,
        RUNNING_CORE);
}

#elif defined ARDUPROF_MBED && defined ARDUINO_ARCH_MBED_RP2040
////////////////////////////////////////////////////////////////////////////////////////////
// Thread for MBed RP2040
////////////////////////////////////////////////////////////////////////////////////////////
#define THREAD_QUEUE_SIZE (128 * EVENTS_EVENT_SIZE) // message queue size for app thread

/////////////////////////////////////////////////////////////////////////////
// use static threadQueue instead of heap
static events::EventQueue threadQueue(THREAD_QUEUE_SIZE);
ThreadApp::ThreadApp() : ardumbedos::ThreadBase(&threadQueue),
                         _handlerMap()
/////////////////////////////////////////////////////////////////////////////
// threadQueue is dynamically allocate from heap
// ThreadApp::ThreadApp() : ThreadBase(THREAD_QUEUE_SIZE),
//                          _handlerMap()
/////////////////////////////////////////////////////////////////////////////
{
    _handlerMap = {
        __EVENT_MAP(ThreadApp, EventNull), // {EventNull, &ThreadApp::handlerEventNull},
    };
}

void ThreadApp::start(void *ctx)
{
    // LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    ThreadBase::start(ctx);
}

#endif

void ThreadApp::setup(void)
{
    ThreadBase::setup();
}

/////////////////////////////////////////////////////////////////////////////
void ThreadApp::onMessage(const Message &msg)
{
    // LOG_TRACE("event=", msg.event, ", iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    auto func = _handlerMap[msg.event];
    if (func)
    {
        (this->*func)(msg);
    }
    else
    {
        LOG_TRACE("Unsupported event=", msg.event, ", iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    }
}

/////////////////////////////////////////////////////////////////////////////
// define EventNull handler
__EVENT_FUNC_DEFINITION(ThreadApp, EventNull, msg) // void ThreadApp::handlerEventNull(const Message &msg)
{
    LOG_TRACE("EventNull(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
}
/////////////////////////////////////////////////////////////////////////////