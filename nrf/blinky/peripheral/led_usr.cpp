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
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

namespace led_usr {
static const struct gpio_dt_spec led =
    GPIO_DT_SPEC_GET(DT_ALIAS(ledusr), gpios);

// Return values
// 0	If successful.
// -ENOTSUP	if any of the configuration options is not supported (unless
// otherwise directed by flag documentation). -EINVAL	Invalid argument. -EIO
// I/O error when accessing an external GPIO chip. -EWOULDBLOCK	if operation
// would block.
int init(void) {
    if (!gpio_is_ready_dt(&led)) {
        printk("Error: GPIO devices not ready\n");
        return -EIO;
    }
    return gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
}

// Return values
// 0	If successful.
// -EIO	I/O error when accessing an external GPIO chip.
// -EWOULDBLOCK	if operation would block.
int set(bool state) { return gpio_pin_set_dt(&led, state); }
} // namespace led_usr
