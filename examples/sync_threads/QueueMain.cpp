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
#include "./QueueMain.h"
#include "./AppContext.h"

////////////////////////////////////////////////////////////////////////////////////////////
QueueMain *QueueMain::_instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
////////////////////////////////////////////////////////////////////////////////////////////
// Thread for ESP32
////////////////////////////////////////////////////////////////////////////////////////////

#define TASK_QUEUE_SIZE 128 // message queue size for app task
static uint8_t ucQueueStorageArea[TASK_QUEUE_SIZE * sizeof(Message)];
static StaticQueue_t xStaticQueue;

////////////////////////////////////////////////////////////////////////////////////////////
void QueueMain::printChipInfo(void)
{
    PRINTLN("===============================================================================");
    PRINTLN("ESP.getChipModel()=", ESP.getChipModel(), ", getChipRevision()=", ESP.getChipRevision(), ", getFlashChipSize()=", ESP.getFlashChipSize(),
            "\r\nNumber of cores=", ESP.getChipCores(), ", SDK version=", ESP.getSdkVersion());
    PRINTLN("ArduProf version: ", ARDUPROF_VER);
    PRINTLN("===============================================================================");
}

QueueMain::QueueMain() : ardufreertos::MessageBus(TASK_QUEUE_SIZE, ucQueueStorageArea, &xStaticQueue),
                         handlerMap()
{
    _instance = this;

    handlerMap = {
        __EVENT_MAP(QueueMain, EventNull), // {EventNull, &QueueMain::handlerEventNull},
    };
}
#endif

void QueueMain::start(void *ctx)
{
    // LOG_TRACE("on core ", xPortGetCoreID(), ", xPortGetFreeHeapSize()=", xPortGetFreeHeapSize());
    MessageBus::start(ctx);

    printChipInfo();
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
        LOG_TRACE("Unsupported event=", msg.event, ", iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    }
}

/////////////////////////////////////////////////////////////////////////////
// define EventNull handler
__EVENT_FUNC_DEFINITION(QueueMain, EventNull, msg) // void QueueMain::handlerEventNull(const Message &msg)
{
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
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
