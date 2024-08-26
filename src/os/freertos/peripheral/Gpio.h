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

#elif defined ESP_PLATFORM
///////////////////////////////////////////////////////////////////////////////
// ESP32
///////////////////////////////////////////////////////////////////////////////
#include "driver/gpio.h"

#ifndef PIN_BITMASK
#define PIN_BITMASK(gpio) (1ULL << gpio)
#endif

class Gpio
{
public:
    Gpio(gpio_num_t pin,
         gpio_mode_t mode = GPIO_MODE_DISABLE,
         uint32_t initialValue = 0) : _PIN(pin),
                                      _ioMode(mode),
                                      _value(initialValue),
                                      _isIntrEnable(false)
    {
        if (mode == GPIO_MODE_OUTPUT || mode == GPIO_MODE_OUTPUT_OD)
        {
            write(_value);
        }
        pinMode(mode);
    }

    ~Gpio()
    {
        detachIntr();
        esp_err_t err = gpio_reset_pin(_PIN);
        UNUSED(err);
    }

    void pinMode(gpio_mode_t ioMode)
    {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = ioMode;
        io_conf.pin_bit_mask = PIN_BITMASK(_PIN);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        esp_err_t err = gpio_config(&io_conf);
        // esp_err_t err = gpio_set_direction(_PIN, ioMode);
        // err
        // - ESP_OK Success
        // - ESP_ERR_INVALID_ARG GPIO error

        UNUSED(err);

        _ioMode = ioMode;
    }

    int read(void)
    {
        return gpio_get_level(_PIN);
    }

    void write(uint32_t value)
    {
        _value = value;
        esp_err_t err = gpio_set_level(_PIN, value);
        UNUSED(err);
    }

    // typedef enum {
    // 	GPIO_INTR_DISABLE = 0,     // Disable GPIO interrupt
    // 	GPIO_INTR_POSEDGE = 1,     // GPIO interrupt type : rising edge
    // 	GPIO_INTR_NEGEDGE = 2,     // GPIO interrupt type : falling edge
    // 	GPIO_INTR_ANYEDGE = 3,     // GPIO interrupt type : both rising and falling edge
    // 	GPIO_INTR_LOW_LEVEL = 4,   // GPIO interrupt type : input low level trigger
    // 	GPIO_INTR_HIGH_LEVEL = 5,  // GPIO interrupt type : input high level trigger
    // 	GPIO_INTR_MAX,
    // } gpio_int_type_t;
    void attachIntr(gpio_int_type_t intrMode, void (*isr)(void *))
    {
        if (!_isIntrEnable && isr)
        {
            _isIntrEnable = true;
            esp_err_t err = gpio_isr_handler_add(_PIN, isr, (void *)this);
            // err:
            // - ESP_OK Success
            // - ESP_ERR_INVALID_STATE Wrong state, the ISR service has not been initialized.
            // - ESP_ERR_INVALID_ARG Parameter error

            err = gpio_set_intr_type(_PIN, intrMode);
            // 	err:
            // 	- ESP_OK  Success
            // 	- ESP_ERR_INVALID_ARG Parameter error

            UNUSED(err);
        }
    }

    void attachIntr(gpio_int_type_t intrMode, void (*isr)(void *), void *arg)
    {
        if (!_isIntrEnable && isr)
        {
            _isIntrEnable = true;
            esp_err_t err = gpio_isr_handler_add(_PIN, isr, arg);
            err = gpio_set_intr_type(_PIN, intrMode);

            UNUSED(err);
        }
    }

    void detachIntr(void)
    {
        if (_isIntrEnable)
        {
            _isIntrEnable = false;
            esp_err_t err = gpio_isr_handler_remove(_PIN);
            // err
            // - ESP_OK Success
            // - ESP_ERR_INVALID_STATE Wrong state, the ISR service has not been initialized.
            // - ESP_ERR_INVALID_ARG Parameter error

            UNUSED(err);
        }
    }

    gpio_num_t getPin(void)
    {
        return _PIN;
    }

protected:
    const gpio_num_t _PIN;
    gpio_mode_t _ioMode;
    uint32_t _value;

    bool _isIntrEnable;
};

///////////////////////////////////////////////////////////////////////////////
#endif