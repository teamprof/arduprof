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
#include "./MessageQueue.h"

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
class MessageBus : public MessageQueue
{
public:
    MessageBus(QueueHandle_t queue) : _isDone(false),
                                      _context(nullptr),
                                      MessageQueue(queue)
    {
    }

    MessageBus(uint16_t queueLength,
               uint8_t *pucQueueStorageBuffer = nullptr,
               StaticQueue_t *pxQueueBuffer = nullptr) : _isDone(false),
                                                         _context(nullptr),
                                                         MessageQueue(queueLength,
                                                                      pucQueueStorageBuffer,
                                                                      pxQueueBuffer)
    {
    }

    virtual void start(void *context)
    {
        _context = context;
    }

    virtual void onMessage(const Message &msg) = 0;

    virtual void messageLoop(int ms = -1)
    // virtual void messageLoop(TickType_t xTicksToWait = portMAX_DELAY)
    {
        configASSERT(_queue);

        TickType_t xTicksToWait = (ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(ms);
        Message msg;
        if (xQueueReceive(_queue, (void *)&msg, xTicksToWait) == pdTRUE)
        {
            onMessage(msg);
        }
        else
        {
            // LOG_TRACE("xQueueReceive() timeout");
        }
    }

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

    // private:
    bool _isDone;
};
#endif