#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

LOCAL os_timer_t blink_timer;
LOCAL uint8_t led_state = 0;

LOCAL void ICACHE_FLASH_ATTR blink_cb() {
  led_state = !led_state;
  GPIO_OUTPUT_SET(4, led_state);
  if (led_state) {
    os_printf("on\r\n");
  } else {
    os_printf("off\r\n");
  }
}

void ICACHE_FLASH_ATTR user_init() {
  gpio_init();
  uart_div_modify(0, UART_CLK_FREQ / 115200);

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); 

  os_timer_disarm(&blink_timer);
  os_timer_setfn(&blink_timer, (os_timer_func_t *)blink_cb, NULL);
  os_timer_arm(&blink_timer, 500, 1);

  wifi_station_set_auto_connect(false);
}
