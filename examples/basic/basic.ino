/* Copyright 2025 teamprof.net@gmail.com
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

/*
  Example of serial log, running on ESP32S3
    [TRACE] QueueMain.cpp L.102 start : on core  1 , xPortGetFreeHeapSize()= 346456
    ===============================================================================
    App Firmware version= 0.1.1
    ESP.getChipModel()= ESP32-S3 , getChipRevision()= 1 , getFlashChipSize()= 8388608
    Number of cores= 2 , SDK version= v5.5-1-gb66b5448e0
    PSRAM total size= 0  bytes, PSRAM free size= 0  bytes
    ===============================================================================
    [TRACE] basic.ino L.103 loop : count= 0
    [TRACE] ThreadApp.cpp L.197 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103 loop : count= 1
    [TRACE] ThreadApp.cpp L.197 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103 loop : count= 2
    [TRACE] ThreadApp.cpp L.197 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103 loop : count= 3
    [TRACE] ThreadApp.cpp L.197 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103 loop : count= 4
    [TRACE] ThreadApp.cpp L.197 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103 loop : count= 5


  Example of serial log, running on Pi Pico2 with FreeRTOS
    [TRACE] QueueMain.cpp L.100 start : core 0
    ===============================================================================
    App Firmware version= 0.1.1
    rp2040_chip_version(): 2 , rp2040_rom_version(): 2
    PSRAM total size: 0
    total heap: 500548 , free heap: 492136
    ===============================================================================
    [TRACE] ThreadApp.cpp L.69 start : core 0
    [TRACE] basic.ino L.115 loop : count= 0
    [TRACE] ThreadApp.cpp L.199 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.115 loop : count= 1
    [TRACE] ThreadApp.cpp L.199 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.115 loop : count= 2
    [TRACE] ThreadApp.cpp L.199 handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0


  Example of serial log, running on Pi Pico with MBed OS
    [TRACE] QueueMain.cpp L.104 start : ...
    ===============================================================================
    App Firmware version= 0.1.1
    rp2040_chip_version()= 1 , rp2040_rom_version()= 2
    ===============================================================================
    [TRACE] ThreadApp.cpp L.165 start : ...
    [TRACE] basic.ino L.103 loop : count= 0
    [TRACE] basic.ino L.103 loop : [TRACE] ThreadApp.cpp L.197 handlerEventNull : EventNull( 0 ), iParam= 0 count=, uParam= 0 , lParam= 0 1

    [TRACE] basic.ino L.103 loop : count=[TRACE] ThreadApp.cpp  L.197 handlerEventNull : 2
    EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103 loop : count= [TRACE] ThreadApp.cpp 3L.197 handlerEventNull : EventNull( 0 ), iParam= 0
    , uParam= 0 , lParam= 0
    [TRACE] basic.ino L.103[TRACE] ThreadApp.cpp  loop : count= L.197 4
    handlerEventNull : EventNull( 0 ), iParam= 0 , uParam= 0 , lParam= 0

    Notice that two lines of log may be merged into one line, since threads are running in parallel.
    Please refer to the sync-threads example on using a semaphore for synchronization between threads.
*/
#include "./ArduProfApp.h"
#include "./ThreadApp.h"
#include "./QueueMain.h"
#include "./AppContext.h"

///////////////////////////////////////////////////////////////////////////////
// define variable "queueMain" for Arduino thred to interact with ArduProf framework.
static QueueMain queueMain;

// define variable "threadApp" for application thread. (Define other thread as you need)
static ThreadApp threadApp;

// define variable "context" and initialize pointers to "queueMain" and "threadApp"
static AppContext context = {
    .queueMain = &queueMain,
    .threadApp = &threadApp,
};

///////////////////////////////////////////////////////////////////////////////
void setup()
{
    /////////////////////////////////////////////////////////////////////////////
    // initial virtual serial port to 115200bps
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }
    delay(3000);

    /////////////////////////////////////////////////////////////////////////////
    // set log output to serial port, and init log params such as log_level
    LOG_SET_LEVEL(DefaultLogLevel);
    // LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
    // LOG_SET_LEVEL(DebugLogLevel::LVL_NONE);
    LOG_SET_DELIMITER(" ");
    LOG_ATTACH_SERIAL(Serial);
    /////////////////////////////////////////////////////////////////////////////

    // initialize queueMain
    queueMain.start(&context);

    // start threadApp
    threadApp.start(&context);
}

void loop()
{
    static int count = 0;
    LOG_TRACE("count=", count++);

    // process message in queueMain if available
    queueMain.messageLoop(0); // non-blocking
    // queueMain.messageLoop(); // blocking until event received and proceed
    // auto pQueueMain = static_cast<QueueMain *>(context.queueMain);
    // assert(pQueueMain);
    // pQueueMain->messageLoop(0); // non-blocking
    // pQueueMain->messageLoop();  // blocking until event received and proceed

    // delay 1s
    delay(1000);

    // post an event "EventNull" to threadApp
    // queueMain.postEvent(EventNull);
    queueMain.postEvent(context.threadApp, EventNull);
    // pQueueMain->postEvent(context.threadApp, EventNull);
}