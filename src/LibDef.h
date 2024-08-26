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

#if defined ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////
// v1.0: first release
// v1.2: add namespace freertos
// v1.3: support esp-idf toolchain
#define LIB_MAJOR_VER 1
#define LIB_MINOR_VER 3
////////////////////////////////////////////////////////////////////////////////////////////

#define dim(x) (sizeof(x) / sizeof(x[0]))
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))

static_assert(sizeof(void *) == sizeof(uint32_t), "sizeof(void *) == sizeof(uint32_t)");
static_assert(sizeof(unsigned long) == sizeof(uint32_t), "sizeof(unsigned long) == sizeof(uint32_t)");

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

////////////////////////////////////////////////////////////////////////////////////////////
#ifndef STR_INDIR
#define STR_INDIR(x) #x
#endif

#ifndef STR
#define STR(x) STR_INDIR(x)
#endif

#define ARDUPROF_VER STR(LIB_MAJOR_VER) "." STR(LIB_MINOR_VER)