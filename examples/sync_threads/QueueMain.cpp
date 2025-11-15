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
#include "./QueueMain.h"
#include "./AppContext.h"
#include "./AppVersion.h"

////////////////////////////////////////////////////////////////////////////////////////////
QueueMain *QueueMain::_instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////

#if defined ARDUPROF_FREERTOS
////////////////////////////////////////////////////////////////////////////////////////////
// QueueMain for ESP32, Pico/Pico2 FreeRTOS
////////////////////////////////////////////////////////////////////////////////////////////
#define TASK_QUEUE_SIZE 16 // message queue size for app task
static uint8_t ucQueueStorageArea[TASK_QUEUE_SIZE * sizeof(Message)];
static StaticQueue_t xStaticQueue;

////////////////////////////////////////////////////////////////////////////////////////////
void QueueMain::printChipInfo(void)
{
    PRINTLN("===============================================================================");
    PRINTLN("App Firmware version=", AppVersion::getFirmwareVersionString());
#if defined ARDUINO_ARCH_RP2040
    PRINTLN("rp2040_chip_version():", rp2040_chip_version(), ", rp2040_rom_version():", rp2040_rom_version(),
            "\r\nPSRAM total size:", rp2040.getPSRAMSize(),
            "\r\ntotal heap:", rp2040.getTotalHeap(), ", free heap:", rp2040.getFreeHeap());

#elif defined ESP_PLATFORM
    PRINTLN("ESP.getChipModel()=", ESP.getChipModel(), ", getChipRevision()=", ESP.getChipRevision(), ", getFlashChipSize()=", ESP.getFlashChipSize(),
            "\r\nNumber of cores=", ESP.getChipCores(), ", SDK version=", ESP.getSdkVersion(),
            "\r\nPSRAM total size=", ESP.getPsramSize(), " bytes, PSRAM free size=", ESP.getFreePsram(), " bytes");
#endif
    PRINTLN("===============================================================================");
}

QueueMain::QueueMain() : ardufreertos::MessageBus(TASK_QUEUE_SIZE, ucQueueStorageArea, &xStaticQueue),
                         _handlerMap()
{
    _instance = this;

    _handlerMap = {
        __EVENT_MAP(QueueMain, EventNull), // {EventNull, &QueueMain::handlerEventNull},
    };
}

#elif defined ARDUPROF_MBED
////////////////////////////////////////////////////////////////////////////////////////////
// Thread for RP2040
////////////////////////////////////////////////////////////////////////////////////////////
#define THREAD_QUEUE_SIZE (128 * EVENTS_EVENT_SIZE) // message queue size for app thread

void QueueMain::printChipInfo(void)
{
    PRINTLN("===============================================================================");
    PRINTLN("App Firmware version=", AppVersion::getFirmwareVersionString());
    PRINTLN("rp2040_chip_version()=", rp2040_chip_version(), ", rp2040_rom_version()=", rp2040_rom_version());
    PRINTLN("===============================================================================");
}

/////////////////////////////////////////////////////////////////////////////
// use static threadQueue instead of heap
static events::EventQueue threadQueue(THREAD_QUEUE_SIZE);
QueueMain::QueueMain() : MessageBus(&threadQueue),
                         _handlerMap()
/////////////////////////////////////////////////////////////////////////////
// threadQueue is dynamically allocate from heap
// QueueMain::QueueMain() : MessageQueue(THREAD_QUEUE_SIZE),
//                          _handlerMap()
/////////////////////////////////////////////////////////////////////////////
{
    // setup event handlers
    _handlerMap = {
        __EVENT_MAP(QueueMain, EventNull), // {EventNull, &QueueMain::handlerEventNull},
    };
}
#endif

void QueueMain::start(void *ctx)
{
#if defined ARDUPROF_FREERTOS && defined ARDUINO_ARCH_RP2040
    LOG_TRACE("core", get_core_num());
#elif defined ARDUPROF_FREERTOS && defined ESP_PLATFORM
    LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
#elif defined ARDUPROF_MBED
    LOG_TRACE("...");
#endif

    MessageBus::start(ctx);

    printChipInfo();
}

void QueueMain::onMessage(const Message &msg)
{
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
__EVENT_FUNC_DEFINITION(QueueMain, EventNull, msg) // void QueueMain::handlerEventNull(const Message &msg)
{
#if defined ARDUPROF_FREERTOS
    auto context = reinterpret_cast<AppContext *>(this->_context);
    configASSERT(context->semaphore);
    if (xSemaphoreTake(context->semaphore, portMAX_DELAY) != pdTRUE)
    {
        return;
    }
    LOG_TRACE("EventNull(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    if (xSemaphoreGive(context->semaphore) != pdTRUE)
    {
        LOG_TRACE("xSemaphoreGive failed!");
    }
#endif
}
/////////////////////////////////////////////////////////////////////////////
