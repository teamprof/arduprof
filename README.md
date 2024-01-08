## ArduProf framework
The ArduProf lib library provides a thin layer framework that makes it easier for developers to code inter-thread communication by an event driven method.  
version 1.0.0 supports FreeRTOS on ESP32.  
version 2.0.0 (plan) supports Mbed OS on Raspherry Pi Pico (RP2040)

[![License: GPL v3](https://img.shields.io/badge/License-GPL_v3-blue.svg)](https://github.com/teamprof/ArduProf/blob/main/LICENSE)

<a href="https://www.buymeacoffee.com/teamprof" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 28px !important;width: 108px !important;" ></a>

---
## supported Hardware (ESP32, ESP32-S3, ESP32C3)
The following boards are supported by this project:
- [ESP32-DevKitC V1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html)
- [ESP32-S3-DevKitC-1 v1.1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html)
- [ESP32C3-CORE](https://wiki.luatos.com/chips/esp32c3/board.html)
- (v2.0.0 plan) [Raspberry Pi Pico](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html)
- (v2.0.0 plan) [W5100S-EVB-Pico](https://www.wiznet.io/product-item/w5100s-evb-pico/)
- (v2.0.0 plan) [WizFi360-EVB-Pico](https://www.wiznet.io/product-item/wizfi360-evb-pico/)


---
### ArduProf framework diagram
[![framework diagram](/doc/image/framework-diagram.png)](https://github.com/teamprof/ArduProf/blob/main/image/framework-diagram.png)


---
## Software setup for ESP32, ESP32-S3, ESP32C3
- Install [Arduino IDE 2.2+ for Arduino](https://www.arduino.cc/en/Main/Software)
- Install [Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- Install [Arduino DebugLog](https://www.arduino.cc/reference/en/libraries/debuglog/)
- Install [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- Install [ArduProf](https://github.com/teamprof/ArduProf)

## Software setup for Raspherry Pi Pico (plan)
- Install [Arduino IDE 2.2+ for Arduino](https://www.arduino.cc/en/Main/Software)
- Install [Arduino Mbed OS RP2040 Boards 4.0.6+](https://github.com/arduino/ArduinoCore-mbed)
- Install [Arduino DebugLog](https://www.arduino.cc/reference/en/libraries/debuglog/)
- Install [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- Install [ArduProf](https://github.com/teamprof/ArduProf)
---


## Build demo code (basic example for ESP32C3)
- launch the Arduino IDE
- create a new project of example "basic" by clicking Menu -> "File" -> "Examples" -> "ArduProf" -> "basic"  
[![New example - basic](/doc/image/example-basic-new.png)](https://github.com/teamprof/ArduProf/blob/main/image/example-basic-new.png)
- select ESP32C3 Dev Module by clicking Menu -> “Tools” -> “Select Board” -> "ESP32C3 Dev Module"  
- config ESP32C3 Dev Module settings as below  
[![Config example - basic](/doc/image/example-basic-esp32c3-settings.png)](https://github.com/teamprof/ArduProf/blob/main/image/example-basic-esp32c3-settings.png)
- build the code by clicking Menu -> “Sketch” -> “Compile/Verify”  
If everything goes smoothly, you should see the following screen.
[![example-basic-build](/doc/image/example-basic-build.png)](https://github.com/teamprof/ArduProf/blob/main/image/example-basic-build.png)



## Flash and run demo code (basic example for ESP32C3)
- select the ESP32C3 virtual port by clicking Menu -> “Tools” -> “Port” -> ...
- build the code by clicking Menu -> “Sketch” -> “Upload”  
If everything goes smoothly, you should see the following screen.
[![example-basic-upload](/doc/image/example-basic-upload.png)](https://github.com/teamprof/ArduProf/blob/main/image/example-basic-upload.png)

Serial monitor log output  
[![example-basic-serial-log](/doc/image/example-basic-serial-log.png)](https://github.com/teamprof/ArduProf/blob/main/image/example-basic-serial-log.png)



## Example code explanation (basic example)
Message format: Format of event message between threads is defined in the file "src/type/Message.h".  
It includes fields such as "event" (signed word), "iParam" (signed word), "uParam" (unsigned word) and "lParam" (unsigned double word)
```
    typedef struct _Message
    {
        int16_t event;
        int16_t iParam;
        uint16_t uParam;
        uint32_t lParam;
    } Message;
```

Example 1: post event from Arduino loop to ThreadApp  
The first step to use ArduProf framework is including the "ArduProf.h" header file. Then defines "queueMain" and "threadApp" and starts them. Posting an event "EventNull" from "queueMain" to "threadApp" is simply as "queueMain.postEvent(context.threadApp, EventNull)"
Sample code is listed in file "basic.ino"
```
#include <ArduProf.h>
...

// define variable "queueMain" for Arduino thred to interact with ArduProf framework.
static QueueMain queueMain;

// define variable "threadApp" for application thread. (Define other thread as you need)
static ThreadApp threadApp;

// define variable "context" and initialize pointers to "queueMain" and "threadApp"
static AppContext context = {
    .queueMain = &queueMain,
    .threadApp = &threadApp,
};

void setup()
{
    ...
    
    // initialize queueMain
    queueMain.start(&context);

    // start threadApp
    threadApp.start(&context);
}

void loop()
{
    ...

    // delay 1s
    delay(1000);

    // post an event "EventNull" to threadApp
    queueMain.postEvent(context.threadApp, EventNull);
}

```

Example 2: handling event "EventNull" in ThreadApp 
There are 3 steps to handle an event. They are:
1. declare event handler
2. define event handler 
3. setup event handler (map event to handler)

Sample code of declare event handler in "ThreadApp.h"
```
class ThreadApp : public ThreadBase
{
    ...

    ///////////////////////////////////////////////////////////////////////
    // declare event handler
    ///////////////////////////////////////////////////////////////////////
    __EVENT_FUNC_DECLARATION(EventNull) // void handlerEventNull(const Message &msg);

}
```

Sample code of define event handler in "ThreadApp.cpp"
```
// define EventNull handler
__EVENT_FUNC_DEFINITION(QueueMain, EventNull, msg) // void QueueMain::handlerEventNull(const Message &msg)
{
    LOG_TRACE("EventNull(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
}
```

Sample code of setup event handler (map event to handler) in "ThreadApp.cpp"
```
ThreadApp::ThreadApp() : ...
{
    ...

    // setup event handlers
    handlerMap = {
        __EVENT_MAP(ThreadApp, EventNull), // {EventNull, &ThreadApp::handlerEventNull},
    };
}
```



---
### Debug
Enable or disable log be modifying macro on "src/LibLog.h"

Debug is disabled by "#define DEBUGLOG_DISABLE_LOG"
Enable trace debug by "#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE"

Example of src/LibLog.h
```
// Disable Logging Macro (Release Mode)
// #define DEBUGLOG_DISABLE_LOG
// You can also set default log level by defining macro (default: INFO)
// #define DEBUGLOG_DEFAULT_LOG_LEVEL_WARN // for release version
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE // for debug version
#include <DebugLog.h>                    // https://github.com/hideakitai/DebugLog
```
---
### Troubleshooting
If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the project will only work if you update the board core to the latest version because I am using newly added functions.

---
### Issues
Submit issues to: [ArduProf issues](https://github.com/teamprof/ArduProf/issues) 

---
### TO DO
- Search for bug and improvement.
- Support Raspberry Pi Pico (RP2040) for version 2.0.0
---

### Contributions and Thanks
Many thanks to the following authors who have developed great audio data and Arduino libraries.
- [DebugLog](https://github.com/hideakitai/DebugLog)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this project.  

---

### Contributing
If you want to contribute to this project:

- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library
---

### License
- The project is licensed under GNU GENERAL PUBLIC LICENSE Version 3
---

### Copyright
- Copyright 2024 teamprof.net@gmail.com. All rights reserved.

