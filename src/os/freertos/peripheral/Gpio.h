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

class Gpio
{
public:
    Gpio(uint8_t pin,
         uint8_t mode = INPUT,
         uint8_t initialValue = LOW) : _PIN(pin),
                                       _ioMode(mode),
                                       _value(initialValue),
                                       _isIntrEnable(false)
    {
        if (mode == OUTPUT || mode == OUTPUT_OPEN_DRAIN)
        {
            digitalWrite(_PIN, initialValue);
        }
        pinMode(mode);
    }

    ~Gpio()
    {
        // esp_err_t rst = gpio_reset_pin(_PIN);
    }

    void pinMode(uint8_t ioMode)
    {
        _ioMode = ioMode;
        ::pinMode(_PIN, ioMode);
    }

    int read(void)
    {
        return digitalRead(_PIN);
    }

    void write(uint8_t value)
    {
        _value = value;
        digitalWrite(_PIN, value);
    }

    void attachIntr(uint8_t intrMode, void (*isr)(void))
    {
        if (!_isIntrEnable)
        {
            attachInterrupt(digitalPinToInterrupt(_PIN), isr, intrMode);
            _isIntrEnable = true;
        }
    }

    void attachIntr(uint8_t intrMode, void (*isr)(void *), void *arg)
    {
        if (!_isIntrEnable)
        {
            attachInterruptArg(digitalPinToInterrupt(_PIN), isr, arg, intrMode);
            _isIntrEnable = true;
        }
    }

    void detachIntr(void)
    {
        if (_isIntrEnable)
        {
            detachInterrupt(digitalPinToInterrupt(_PIN));
            _isIntrEnable = false;
        }
    }

    uint8_t getPin(void)
    {
        return _PIN;
    }

protected:
    const uint8_t _PIN;
    uint8_t _ioMode;
    uint8_t _value;

    bool _isIntrEnable;
};
