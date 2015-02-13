# esp8266_easyintr
Easy Interrupt Management for ESP8266

As the SDK allows only ONE gpio interrupt handler a little dispatcher can help with code reuse.

Usage:
```
int main(...) {
	/* Call ei_init early before modules using it  */
	ei_init();
	/* Initialize some module */
	module_init();
}


void module_isr(void *data)
{
	int *p = data;
	if (*p == 42) {
		os_printf("Tasty %d!\n", *p);
	}
}

#define GPIO_NUM 0
#define GPIO_MUX PERIPHS_IO_MUX_GPIO0_U
#define GPIO_FUNC FUNC_GPIO0

void module_init()
{
	int *module_data = os_malloc(sizeof(int));
	*module_data = 42;
	/* Install module ISR routine */
	ei_attach(GPIO_NUM, GPIO_MUX, GPIO_FUNC, module_isr, module_data);
}

void module_dont_bother_me()
{
	/* Do not call module ISR any more */
	ei_detach(GPIO_NUM);
}
```

