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

#if ARDUINO
///////////////////////////////////////////////////////////////////////////////
// Arduino
///////////////////////////////////////////////////////////////////////////////
// Disable Logging Macro (Release Mode)
// #define DEBUGLOG_DISABLE_LOG
// You can also set default log level by defining macro (default: INFO)
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
// #include "../../DebugLog/DebugLog.h" // https://github.com/hideakitai/DebugLog
#include <DebugLog.h> // https://github.com/hideakitai/DebugLog

#define DefaultLogLevel (DebugLogLevel::LVL_TRACE)

#elif defined ESP_PLATFORM
///////////////////////////////////////////////////////////////////////////////
// ESP32/ESP32S3/ESP32C3
///////////////////////////////////////////////////////////////////////////////
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

///////////////////////////////////////////////////////////////////////////////
#endif
