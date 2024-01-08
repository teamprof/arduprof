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
#include <Arduino.h>
#include "../../type/Message.h"

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
class MessageQueue
{
public:
    MessageQueue(QueueHandle_t queue) : _queue(queue)
    {
    }

    MessageQueue(uint16_t queueLength,
                 uint8_t *pucQueueStorageBuffer = nullptr,
                 StaticQueue_t *pxQueueBuffer = nullptr)
    {
        if (pucQueueStorageBuffer != nullptr && pxQueueBuffer != nullptr)
        {
            _queue = xQueueCreateStatic(queueLength, sizeof(Message), pucQueueStorageBuffer, pxQueueBuffer);
        }
        else
        {
            _queue = xQueueCreate(queueLength, sizeof(Message));
        }
        configASSERT(_queue != NULL);
    }

    ~MessageQueue()
    {
        vQueueDelete(_queue);
        _queue = nullptr;
    }

    void postEvent(MessageQueue *msgQueue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, TickType_t xTicksToWait = 0)
    {
        Message msg = {
            .event = event,
            .iParam = iParam,
            .uParam = uParam,
            .lParam = lParam,
        };
        postEvent(msgQueue, msg);
    }
    void postEvent(MessageQueue *msgQueue, const Message &msg, TickType_t xTicksToWait = 0)
    {
        if (msgQueue && msgQueue->_queue)
        {
            if (xPortInIsrContext())
            // if (xPortIsInsideInterrupt())
            {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                if (xQueueSendFromISR(msgQueue->_queue, &msg, &xHigherPriorityTaskWoken) != pdTRUE)
                {
                    // LOG_ERROR("xQueueSend failed!");
                }
                portYIELD_FROM_ISR();
                // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            else
            {
                if (xQueueSend(msgQueue->_queue, &msg, xTicksToWait) != pdTRUE)
                // if (xQueueSend(msgQueue->queue, &msg, portMAX_DELAY) != pdTRUE)
                {
                    // LOG_ERROR("xQueueSend failed!");
                }
            }
        }
    }

    inline void postEvent(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L, TickType_t xTicksToWait = 0)
    {
        postEvent(this, event, iParam, uParam, lParam, xTicksToWait);
    }
    inline void postEvent(const Message &msg, TickType_t xTicksToWait = 0)
    {
        postEvent(this, msg, xTicksToWait);
    }

    void sendMessageToTask(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
    {
        if (_queue == nullptr)
        {
            return;
        }

        Message msg = {
            .event = event,
            .iParam = iParam,
            .uParam = uParam,
            .lParam = lParam,
        };
        if (xQueueSend(_queue, &msg, 0) != pdTRUE)
        // if (xQueueSend(queue, &msg, portMAX_DELAY) != pdTRUE)
        {
            // DBGLOG(Debug, "%s - sendMessageToTask failed!", TAG);
        }
    }

    void sendMessageFromIsrToTask(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
    {
        if (_queue == nullptr)
        {
            return;
        }

        Message msg = {
            .event = event,
            .iParam = iParam,
            .uParam = uParam,
            .lParam = lParam,
        };
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(_queue, &msg, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR();
    }

    inline QueueHandle_t queue(void)
    {
        return _queue;
    }

protected:
    QueueHandle_t _queue;
};

/////////////////////////////////////////////////////////////////////////////
#define __EVENT_MAP(class, event)      \
    {                                  \
        event, &class ::handler##event \
    }
#define __EVENT_FUNC_DEFINITION(class, event, msg) void class ::handler##event(const Message &msg)
#define __EVENT_FUNC_DECLARATION(event) void handler##event(const Message &msg);
/////////////////////////////////////////////////////////////////////////////

#endif