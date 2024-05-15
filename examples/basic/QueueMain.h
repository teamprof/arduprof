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
#include <map>
#include <ArduProf.h>

#include "./AppEvent.h"

class QueueMain final : public ardufreertos::MessageBus
{
public:
    QueueMain();

    virtual void start(void *);
    virtual void onMessage(const Message &msg) override;

    static void printChipInfo(void);

protected:
    typedef void (QueueMain::*funcPtr)(const Message &);
    std::map<int16_t, funcPtr> handlerMap;

private:
    static QueueMain *_instance;

    ///////////////////////////////////////////////////////////////////////
    // declare event handler
    ///////////////////////////////////////////////////////////////////////
    __EVENT_FUNC_DECLARATION(EventNull) // void handlerEventNull(const Message &msg);
};
