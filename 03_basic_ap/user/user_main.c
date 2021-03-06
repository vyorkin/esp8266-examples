#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

LOCAL uint32_t chip_id;

LOCAL bool connected = false;
LOCAL uint8_t connected_devices = 0;
LOCAL uint8_t led_state = 0;

LOCAL os_timer_t loop_timer;

void ICACHE_FLASH_ATTR wifi_handle_event(System_Event_t *e) {
  os_printf("[INFO] ");

  switch (e->event) {
    case EVENT_STAMODE_CONNECTED:
      os_printf(
        "connected to %s channel %d\n", 
        e->event_info.connected.ssid, 
        e->event_info.connected.channel
      );
      break;
    case EVENT_STAMODE_DISCONNECTED:
      os_printf(
        "disconnected from %s, due to code :%d\n", 
        e->event_info.disconnected.ssid, 
        e->event_info.disconnected.reason
      );
      break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
      os_printf(
        "auth mode updated from %d to %d\n", 
        e->event_info.auth_change.old_mode, 
        e->event_info.auth_change.new_mode
      );
      break;
    case EVENT_STAMODE_GOT_IP:
      connected = true;
      ip_addr_t my_ip = e->event_info.got_ip.ip;
      os_printf(
        "ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, 
        IP2STR(&my_ip),
        IP2STR(&e->event_info.got_ip.mask),
        IP2STR(&e->event_info.got_ip.gw)
      );
      os_printf("\n");
      break;
    case EVENT_OPMODE_CHANGED:
      os_printf(
        "op mode changed from %d to %d \n",
        e->event_info.opmode_changed.old_opmode,
        e->event_info.opmode_changed.new_opmode
      );
      break;
    case EVENT_SOFTAPMODE_STACONNECTED:
      connected_devices++;
      os_printf(
        "station: " MACSTR " join, AID = %d\n",
        MAC2STR(e->event_info.sta_connected.mac),
        e->event_info.sta_connected.aid
      );
      break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
      connected_devices--;
      os_printf(
        "station: " MACSTR " leave, AID = %d\n",
        MAC2STR(e->event_info.sta_disconnected.mac),
        e->event_info.sta_disconnected.aid
      );
      break;
    case EVENT_SOFTAPMODE_PROBEREQRECVED:
      os_printf(
        "probe request received from " MACSTR ", RSSI = %d\n",
        MAC2STR(e->event_info.ap_probereqrecved.mac),
        e->event_info.ap_probereqrecved.rssi
      );
      break;
    default:
      os_printf("Unexpected event: %d\n", e->event);
      break;
  }
}

static void ICACHE_FLASH_ATTR setup() {
  os_printf("[INFO] sdk version: %s\n", system_get_sdk_version());
  chip_id = system_get_chip_id();
  os_printf("[INFO] chip ID: 0x%x\n", chip_id);
  os_printf("[INFO] system started\n");

  if (!wifi_station_set_auto_connect(false)) {
    os_printf("[ERR] wifi_station_set_auto_connect: fail\n");
  }
  wifi_set_event_handler_cb(wifi_handle_event);

  if (!wifi_set_opmode_current(SOFTAP_MODE)) {
    os_printf("[ERR] wifi_set_opmode_current: fail\n");
  }

  struct softap_config cfg;
  os_memset(&cfg, 0, sizeof(cfg)); 

  os_strcpy(cfg.ssid, SSID);

  cfg.ssid_len = os_strlen(SSID);
  cfg.authmode = AUTH_OPEN;
  cfg.ssid_hidden = 0;
  cfg.max_connection = 4;

  if (!wifi_softap_set_config_current(&cfg)) {
    os_printf("[ERR] wifi_softap_set_config_current: fail\n");
  }
}

void ICACHE_FLASH_ATTR loop() {
  led_state = !led_state;
  GPIO_OUTPUT_SET(4, led_state);
}

void ICACHE_FLASH_ATTR user_init() {
  gpio_init();
  uart_div_modify(0, UART_CLK_FREQ / 115200);

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); 

  os_timer_disarm(&loop_timer);
  os_timer_setfn(&loop_timer, (os_timer_func_t *)loop, NULL);
  os_timer_arm(&loop_timer, 200, 1);

  system_init_done_cb(setup);
}
