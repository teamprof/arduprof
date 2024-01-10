/* Copyright 2023 teamprof.net@gmail.com
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
#include <ArduinoJson.h>

template <size_t desiredCapacity>
class JsonMessage : public StaticJsonDocument<desiredCapacity>
{
public:
    JsonMessage(Stream *stream = nullptr) : _stream(stream)
    {
    }

    size_t serialize(int16_t event, int16_t arg0 = 0, uint16_t arg1 = 0,
                     const uint8_t *data = nullptr, uint16_t length = 0,
                     Stream *stream = nullptr)
    {
        this->clear();
        (*this)["event"] = event;
        (*this)["arg0"] = arg0;
        (*this)["arg1"] = arg1;
        if (data)
        {
            JsonArray jsonData = this->createNestedArray("data");
            for (uint16_t i = 0; i < length; i++)
            {
                jsonData.add((uint8_t)(data[i]));
            }
        }
        if (stream)
        {
            return serializeJson(*this, *stream);
        }
        else if (this->_stream)
        {
            return serializeJson(*this, *this->_stream);
        }
        else
        {
            return 0;
        }
    }

    size_t serialize(Stream *stream = nullptr)
    {
        if (stream == nullptr)
        {
            stream = _stream;
        }
        // configASSERT(stream);

        size_t size = serializeJson(*this, *stream);
        stream->println();
        return size;
    }

    DeserializationError deserialize(Stream *stream = nullptr)
    {
        if (stream == nullptr)
        {
            stream = _stream;
        }
        // configASSERT(stream);
        return deserializeJson(*this, *stream);
    }

    DeserializationError deserialize(const char *message)
    {
        // LOG_TRACE("  message = ", message);
        this->clear();
        return deserializeJson(*this, message);
    }

    Stream *stream(void)
    {
        return _stream;
    }

private:
    Stream *_stream;
};
