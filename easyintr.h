#ifndef __EASY_INTR_H__
#define __EASY_INTR_H__

/* Initiallize easy interrupts */
void ei_init(void);
/* Detach interrupt handler */
bool ei_detach(uint8_t gpio_pin);
/* Install GPIO interrupt handler */
void ei_attach(uint8 gpio_pin, uint32 gpio_name, uint8 gpio_func, void (*vect)(void *), void *data);


#endif
