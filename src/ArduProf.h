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
#include "./LibDef.h"
#include "./LibLog.h"

#include "./type/Message.h"
#include "./type/JsonMessage.h"

#if !defined ARDUPROF_MBED && !defined ARDUPROF_FREERTOS
#if defined ARDUINO_ARCH_ESP32
#define ARDUPROF_FREERTOS
#elif ARDUINO_ARCH_MBED_RP2040
#define ARDUPROF_MBED
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
// For FreeRTOS
#if defined ARDUPROF_FREERTOS

#if defined ARDUINO_ESP32_DEV || ARDUINO_ESP32C3_DEV || defined ARDUINO_XIAO_ESP32C3
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#else
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#endif

#include "./os/freertos/thread/ThreadBase.h"
#include "./os/freertos/peripheral/PeriodicTimer.h"
#include "./os/freertos/peripheral/SoftwareTimer.h"
#include "./os/freertos/peripheral/Gpio.h"

///////////////////////////////////////////////////////////////////////////////
// For Zephyr
#elif defined ARDUPROF_ZEPHYR
#include "./os/zephyr/MessageQueue.h"
#include "./os/zephyr/MessageBus.h"
#include "./os/zephyr/thread/ThreadBase.h"

///////////////////////////////////////////////////////////////////////////////
// For MBED
#elif defined ARDUPROF_MBED
#include "./os/mbed/thread/ThreadBase.h"
#include "./os/mbed/peripheral/Gpio.h"
#include "./os/mbed/peripheral/PeriodicTimer.h"

///////////////////////////////////////////////////////////////////////////////
#endif
