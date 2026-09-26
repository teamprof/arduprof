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

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <functional>
#include <vector>

#define TASK_POOL_NAME STR(ThreadPool)

#ifndef _BIT
#define _BIT(x) (0x01UL << (x))
#endif

namespace ardufreertos {
class ThreadPool {
public:
    using TaskFunction = std::function<void()>;

    ThreadPool(size_t workerCount, 
        size_t queueSize = 5,
        uint32_t stackSize = 4096, 
        UBaseType_t priority = 1, 
        int core = 0) : isStopping(false) 
    {
        // Create a FreeRTOS queue to hold function pointers
        taskQueue = xQueueCreate(queueSize, sizeof(TaskFunction *));

        // Create worker tasks
        workers.reserve(workerCount);
        for (auto i = 0; i < workerCount; i++) 
        {
            TaskHandle_t handle = NULL;
#if defined ARDUPROF_PICO_C
#if configNUMBER_OF_CORES == 1
            UBaseType_t mask = _BIT(0);
#elif configNUMBER_OF_CORES == 2
            UBaseType_t mask = _BIT(0) | _BIT(1);
#endif
            if(core > 0 && core < configNUMBER_OF_CORES) 
            {
                mask = _BIT(core);
            }
            auto rst = xTaskCreateAffinitySet(
                workerLoop,     // Task function
                TASK_POOL_NAME, // Task name
                stackSize,      // Stack size
                this,           // Parameters
                priority,       // Priority
                mask,           // Core affinity mask
                &handle         // Task handle
            );

#elif defined(ESP_PLATFORM)
            auto rst = xTaskCreatePinnedToCore(
                workerLoop,     // Task function
                TASK_POOL_NAME, // Name
                stackSize,      // Stack depth
                this,           // Parameters passed to task
                priority,       // Priority
                &handle,        // Task handle
                core            // cpu core
            );
#endif
            // auto rst = xTaskCreate(
            //     workerLoop,     // Task function
            //     TASK_POOL_NAME, // Name
            //     stackSize,      // Stack depth
            //     this,           // Parameters passed to task
            //     priority,       // Priority
            //     &handle         // Task handle
            // );
            if (rst != pdTRUE)
            {
        #ifdef ARDUINO
                LOG_WARN("xTaskCreatePinnedToCore() of worker ", i, "failed");
        #endif        
            }
            workers.push_back(handle);
        }
    }

    ~ThreadPool() 
    {
        isStopping = true;

        TaskFunction *taskToRun = nullptr;
        while (xQueueReceive(taskQueue, &taskToRun, 0) == pdTRUE)
        {
            delete taskToRun;
        }

        for (TaskHandle_t handle : workers)
        {
            if (handle != NULL)
            {
                vTaskDelete(handle);
            }
        }

        if (taskQueue != NULL)
        {
        vQueueDelete(taskQueue);
        }
    }

    // Submit a generic lambda or function to the pool
    bool enqueue(TaskFunction task)
    {
        if (isStopping)
        {
            return false;
        }

        // Allocate task function dynamically to pass pointer through FreeRTOS queue
        TaskFunction *taskPtr = new TaskFunction(task);

        // Try putting task in queue (wait max 10ms if full)
        if (xQueueSend(taskQueue, &taskPtr, pdMS_TO_TICKS(10)) != pdTRUE)
        {
            delete taskPtr; // Cleanup if queue is full
            return false;
        }
        return true;
    }

private:
    std::vector<TaskHandle_t> workers;
    QueueHandle_t taskQueue;
    bool isStopping;

    // Static entry point for FreeRTOS tasks
    static void workerLoop(void *arg)
    {
        ThreadPool *pool = static_cast<ThreadPool *>(arg);
        TaskFunction *taskToRun = nullptr;

        while (true)
        {
            // Block indefinitely until a task is available in the queue
            if (xQueueReceive(pool->taskQueue, &taskToRun, portMAX_DELAY) == pdTRUE)
            {
                if (taskToRun && *taskToRun)
                {
                    (*taskToRun)();   // Execute the task
                    delete taskToRun; // Free dynamically allocated task
                }
            }

            if (pool->isStopping)
            {
                vTaskDelete(NULL); // Terminate current FreeRTOS task
            }
        }
    }
};
}; // namespace ardufreertos
