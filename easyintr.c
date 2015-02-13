/*
* easyintr.c
*
* Copyright (c) 2015, Alexander Atanasov (http://zazolabs.com/)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "easyintr.h"
#include "gpio.h"
#include "osapi.h"
#include "ets_sys.h"

typedef void (*ei_intr_vect)(void *);
struct ei_handler {
	ei_intr_vect isr;
	void *data;
};

static ei_handler ei_vects[15];

static void ICACHE_FLASH_ATTR ei_def_intr(void *data)
{
	int bitno = (int)data;
	os_printf("Unhandled intr:%d\n", bitno);
}

static void ICACHE_FLASH_ATTR ei_install_default(int pin)
{
	ei_vects[pin].isr = ei_def_intr;
	ei_vects[pin].data = (void *)pin;
}

bool ICACHE_FLASH_ATTR ei_detach(uint8_t gpio_pin)
{
	if (gpio_pin == 16) {
		os_printf("ei_detach: GPIO16 not interrupts\n");
		return false;
	}

	gpio_pin_intr_state_set(GPIO_ID_PIN(gpio_pin), GPIO_PIN_INTR_DISABLE);
	ei_install_default(gpio_pin);
	return true;
}


LOCAL void ICACHE_FLASH_ATTR  ei_intr_dispatch(void)
{
	uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	int i;
	int bit;
	for (i=0; i < 15; i++) {
		bit = 1<<i;
		if (gpio_status & bit) {
			// ack the intr
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & bit);
			ei_vects[i].isr(ei_vects[i].data);
		}
	}
}

void ICACHE_FLASH_ATTR ei_init(void)
{
	int i;
	/* Initialize default ISR */
	for (i=0; i < sizeof(ei_vects)/sizeof(ei_vects[0]); i++) {
		ei_vects[i].isr = ei_def_intr;
		ei_vects[i].data = (void *)i;
	}
	ETS_GPIO_INTR_ATTACH(ei_intr_dispatch, NULL);
}


void ICACHE_FLASH_ATTR ei_attach(uint8 gpio_pin, uint32 gpio_name, uint8 gpio_func, void (*vect)(void *), void *data)
{
	ETS_GPIO_INTR_DISABLE();

	ei_vects[i].isr = vect;
	ei_vects[i].data = data;

	PIN_FUNC_SELECT(gpio_name, gpio_func);

        gpio_output_set(0, 0, 0, GPIO_ID_PIN(gpio_pin));

        gpio_register_set(GPIO_PIN_ADDR(gpio_pin), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

        // clear gpio14 status
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio_pin));

        // enable interrupt
        gpio_pin_intr_state_set(GPIO_ID_PIN(gpio_pin), GPIO_PIN_INTR_ANYEGDE);
	ETS_GPIO_INTR_ENABLE();
}
